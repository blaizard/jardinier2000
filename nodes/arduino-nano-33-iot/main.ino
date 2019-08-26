//#include <ArduinoLowPower.h>
#include <Arduino.h>
#include <initializer_list>

#include "include/arduino_secrets.h"
#include "src/log.h"
#include "src/error.h"
#include "src/wifi.h"
#include "src/time.h"
#include "src/system.h"
#include "src/topic.h"
#include "src/array.h"
#include "src/vector.h"
#include "src/string.h"

#include "src/data/dht.h"
#include "src/data/analog.h"

#include <WiFiNINA.h>

struct GeneratorData
{
  node::data::Type m_type;
  node::data::Generator::value_type m_value;
  const char *m_name;
};

// An entry with a timestamp
struct Snapshot
{
  node::time::time_type m_timestamp;
  node::Vector<GeneratorData, 4> m_generators;
};

using Snapshots = node::Vector<Snapshot, 32>;
using Buffer = node::String<1024>;

// Variables
static Snapshots m_snapshots;
static Buffer m_buffer;

// If you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
char server[] = "blaizard.com"; // name address for Google (using DNS)

struct TopicApp : public ::node::Topic
{
  static constexpr const char *toString = "app";
};

void sendData(WiFiSSLClient& client, const char* str)
{
  {
    String line = "POST /jardinier2000/api/v1/sample?token=";
    line += NODE_TOKEN;
    line += " HTTP/1.1";
    client.println(line);
  }
//  client.println("Cookie: irpath=/jardinier2000");
  client.println("Host: blaizard.com");
  client.println("Content-Type: application/json; charset=utf-8");
  {
    String line = "Content-Length: ";
    line += strlen(str);
    client.println(line);
  }
  client.println("Connection: close");
  client.println();
  client.println(str);

  node::log::info<TopicApp>("Sending: ", str);
}

/**
 * Create the json data from the generators
 * 
 * {
 *  "timestamp": 0,
 *  "humidity": {"sensor1": 45, "sensor2": 454},
 *  ...
 * }
 */
void snapshotToJson(Buffer& buffer, const Snapshot &snapshot) noexcept
{
  // Fill the timestamps entry
  buffer += "{\"timestamp\":";
  buffer += snapshot.m_timestamp;

  // Fill the generator data
  for (const auto &supported : node::data::supported)
  {
    bool isFirst = true;
    for (const auto &generator : snapshot.m_generators)
    {
      if (generator.m_type == supported.m_type)
      {
        if (isFirst)
        {
          buffer += ",\"";
          buffer += supported.m_name;
          buffer += "\":{\"";
        }
        else
        {
          buffer += ",\"";
        }

        buffer += generator.m_name;
        buffer += "\":";
        buffer += generator.m_value;

        isFirst = false;
      }
    }
    if (!isFirst)
    {
      buffer += "}";
    }
  }
  buffer += "}";
}

void waitForResponse(WiFiSSLClient &client, int timeoutS)
{
  while (!client.available() && timeoutS > 0)
  {
    delay(1000);
    --timeoutS;
  }

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available())
  {
    char c = client.read();
    Serial.write(c);
  }
}

void takeSnapshot(Snapshots& snapshots)
{
  // Setup the sensors
  node::data::DHT dataDht(PIN_A3);
  node::data::Analog dataPhotoresistor("photoresistor", PIN_A7, node::data::Type::LUMINOSITY, [](const node::data::Generator::value_type value) -> node::data::Generator::value_type {
    return 255 - value;
  });
  node::data::Analog dataMoisture("moisture", PIN_A6, node::data::Type::MOISTURE, [](const node::data::Generator::value_type value) -> node::data::Generator::value_type {
    const auto newValue = 255 - value;
    if (newValue > 127)
    {
      return 255;
    }
    return (newValue << 1);
  });

  node::Array<node::data::Generator::ptr_type, 3> dataGenerators(&dataDht, &dataPhotoresistor, &dataMoisture);

  // Allocate a new entry and get its reference
  snapshots.push_back({.m_timestamp = 0});
  auto &generators = snapshots[snapshots.size() - 1].m_generators;

  for (auto &data : dataGenerators)
  {
    data->start();

    // Gather the data of the generators
    for (const auto &supported : node::data::supported)
    {
      if (data->isSupportedType(supported.m_type))
      {
        generators.push_back({.m_type = supported.m_type,
                              .m_value = data->getValue(supported.m_type),
                              .m_name = data->getName()});
      }
    }

    data->stop();
  }
}

void sendSnapshots(WiFiSSLClient& client, Snapshots& snapshots, Buffer &buffer)
{
  node::size_t index = 0;
  node::size_t previousFullIndex = 0;
  buffer.clear();
  while (index < snapshots.size())
  {
    const size_t previousSize = buffer.size();
    buffer += (buffer.empty()) ? "{\"list\":[" : ",";

    // Fill the buffer
    snapshotToJson(buffer, snapshots[index]);

    // Check if buffer is full and revert or go to next
    const bool isFull = (buffer.size() + 3 > buffer.capacity());
    if (isFull)
    {
      buffer.resize(previousSize);
    }
    else
    {
      ++index;
    }

    // Send the buffer if it is full or it is the last one
    if (isFull || index == snapshots.size())
    {
      node::error::assertTrue<TopicApp>(previousFullIndex < index, "Buffer is too small");

      buffer += "]}";
      sendData(client, buffer.data());
      waitForResponse(client, 30);

      node::log::info<TopicApp>(buffer.data());
      buffer.clear();
      previousFullIndex = index;
    }
  }
}

void saveSnapshots(Snapshots& snapshots, Buffer& buffer)
{
  // Connect to the wifi
  node::wifi::Scope scope(SECRET_SSID, SECRET_PASS);

  // Initialize the Ethernet client library
  // with the IP address and port of the server
  // that you want to connect to (port 80 is default for HTTP):
  WiFiSSLClient client;

  // Connect to server
  {
    constexpr const node::uint8_t maxAttempt = 3;
    node::uint8_t nbAttempt = 0;
    bool isConnected = false;
    do
    {
      node::log::info<TopicApp>("Starting connection to server... (", nbAttempt + 1, "/", maxAttempt, ")");
      isConnected = client.connect(server, 443);
      ++nbAttempt;
    } while (!isConnected && nbAttempt < maxAttempt);
    node::error::assertTrue<TopicApp>(isConnected, "Failed to connect to server");
    node::log::info<TopicApp>("Connected to server");
  }

  // Make a HTTP request
  sendSnapshots(client, snapshots, buffer);

  if (!client.connected())
  {
    node::log::info<TopicApp>("Disconnecting from server");
    client.stop();
  }
}

void setup()
{
}

void loop()
{
  Snapshots& snapshots = m_snapshots;
  Buffer& buffer = m_buffer;

  node::system::start();

  delay(5000);

  takeSnapshot(snapshots);
  saveSnapshots(snapshots, buffer);
  snapshots.clear();

  delay(10000);

  node::system::restartAfter32min();
}
