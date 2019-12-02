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
  node::system::timestamp_type m_timestamp;
  node::Vector<GeneratorData, 4> m_generators;
};

using Snapshots = node::Vector<Snapshot, 32>;
using Buffer = node::String<1024>;

// Variables
static Snapshots m_snapshots;
static Buffer m_buffer;

// If you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
constexpr const char server[] = "blaizard.com"; // name address for Google (using DNS)
constexpr int serverPort = 443; // Server port
constexpr const char serverRoot[] = "/jardinier2000/"; // Server root to access the REST API
#define Client WiFiSSLClient

// For local development only
//constexpr const char server[] = "192.168.2.100";
//constexpr int serverPort = 8001;
//constexpr const char serverRoot[] = "/";
//#define Client WiFiClient

struct TopicApp : public ::node::Topic
{
  static constexpr const char *toString = "app";
};

bool sendData(Client& client, const char* str)
{
  {
    client.print("POST ");
    client.print(serverRoot);
    client.print("api/v1/sample?token=");
    client.print(NODE_TOKEN);
    client.println(" HTTP/1.1");
  }
  {
    client.print("Host: ");
    client.println(server);
  }
  client.println("Content-Type: application/json; charset=utf-8");
  {
    client.print("Content-Length: ");
    client.println(strlen(str));
  }
  client.println("Connection: close");
  client.println();
  client.println(str);

  node::log::info<TopicApp>("Sending: ", str);

  return waitForResponse(client, 30);
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
bool snapshotToJson(Buffer& buffer, const Snapshot &snapshot, const node::system::timestamp_type timestamp) noexcept
{
  // Fill the timestamps entry
  buffer += "{\"timestamp\":";
  buffer += ((timestamp - snapshot.m_timestamp) / 1000);

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

  return true;
}

bool waitForResponse(Client &client, int timeoutS)
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

  return true;
}

bool takeSnapshot(Snapshots& snapshots, const node::system::timestamp_type timestamp)
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
  snapshots.push_back({
    .m_timestamp = timestamp
  });
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

  return true;
}

bool sendSnapshots(Client& client, Snapshots& snapshots, Buffer& buffer, const node::system::timestamp_type timestamp)
{
  node::size_t index = 0;
  node::size_t previousFullIndex = 0;
  buffer.clear();
  while (index < snapshots.size())
  {
    const size_t previousSize = buffer.size();
    buffer += (buffer.empty()) ? "{\"list\":[" : ",";

    // Fill the buffer
    if (!node::error::assertTrue<TopicApp>(snapshotToJson(buffer, snapshots[index], timestamp), "Error while serializing snapshot"))
    {
      return false;
    }

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
      if (!node::error::assertTrue<TopicApp>(previousFullIndex < index, "Buffer is too small"))
      {
        return false;
      }

      buffer += "]}";
      if (!node::error::assertTrue<TopicApp>(sendData(client, buffer.data()), "Error while sending data to server"))
      {
        return false;
      }

      node::log::info<TopicApp>(buffer.data());
      buffer.clear();
      previousFullIndex = index;
    }
  }

  return true;
}

bool saveSnapshots(Snapshots& snapshots, Buffer& buffer, const node::system::timestamp_type timestamp)
{
  // Connect to the wifi
  node::wifi::Scope scope(SECRET_SSID, SECRET_PASS);

  // Initialize the Ethernet client library
  // with the IP address and port of the server
  // that you want to connect to (port 80 is default for HTTP):
  Client client;

  // Connect to server
  {
    constexpr const node::uint8_t maxAttempt = 3;
    node::uint8_t nbAttempt = 0;
    bool isConnected = false;
    do
    {
      node::log::info<TopicApp>("Starting connection to ", server, ":", serverPort, "... (", nbAttempt + 1, "/", maxAttempt, ")");
      isConnected = client.connect(server, serverPort);
      ++nbAttempt;
    } while (!isConnected && nbAttempt < maxAttempt);
    if (!node::error::assertTrue<TopicApp>(isConnected, "Failed to connect to server"))
    {
      return false;
    }
    node::log::info<TopicApp>("Connected to server");
  }

  // Make a HTTP request
  sendSnapshots(client, snapshots, buffer, timestamp);

  if (!client.connected())
  {
    node::log::info<TopicApp>("Disconnecting from server");
    client.stop();
  }

  return true;
}

void setup()
{
  // Used to be able to reflash the app
  delay(5000);

  node::system::start();
}

void loop()
{
  Snapshots& snapshots = m_snapshots;
  Buffer& buffer = m_buffer;

  takeSnapshot(snapshots, node::system::timestamp);
  saveSnapshots(snapshots, buffer, node::system::timestamp);
  delay(5000);
  takeSnapshot(snapshots, node::system::timestamp);
  saveSnapshots(snapshots, buffer, node::system::timestamp);
  delay(5000);
  takeSnapshot(snapshots, node::system::timestamp);
  saveSnapshots(snapshots, buffer, node::system::timestamp);
  delay(5000);

  while (true)
  {
    takeSnapshot(snapshots, node::system::timestamp);

    // Save and clear the snapshots every 2h
    if (snapshots.size() >= 4)
    {
      saveSnapshots(snapshots, buffer, node::system::timestamp);
      snapshots.clear();
    }

    node::system::sleepFor32min();
  }
}
