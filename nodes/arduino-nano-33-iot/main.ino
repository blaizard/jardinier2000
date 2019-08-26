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

#include "src/data/dht.h"
#include "src/data/analog.h"

#include <WiFiNINA.h>

struct Generator
{
  node::DataType m_type;
  node::data::Generator::value_type m_value;
  const char* m_name;
};

// An entry with a timestamp
struct Snapshot
{
  node::time::time_type m_timestamp;
  node::Vector<Generator, 4> m_generators;
};

using Snapshots = node::Vector<Snapshot, 32>;

// If you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
char server[] = "blaizard.com";    // name address for Google (using DNS)

struct TopicApp : public ::node::Topic
{
	static constexpr const char* toString = "app";
};

void sendData(WiFiSSLClient& client, const String& data)
{
  {
    String line = "POST /jardinier2000/api/v1/sample?token=";
    line += NODE_TOKEN;
    line += " HTTP/1.1";
    client.println(line);
  }
  client.println("Cookie: irpath=/jardinier2000");
  client.println("Host: blaizard.com");
  client.println("Content-Type: application/json; charset=utf-8");
  {
    String line = "Content-Length: ";
    line += data.length();
    client.println(line);
  }
  client.println("Connection: close");
  client.println();
  client.println(data);
}

/**
 * Create the json data from the generators
 * 
 * "timestamps": [..., ..., ...],
 * "humidity": [{"sensor1": 45, "sensor2": 454}],
 * ...
 */
String toJson(const Snapshots& snapshots)
{
  String data = "{";

  // Fill the timestamps entry
  {
    data += "\"timestamps\":[";
    for (node::size_t i = 0; i < snapshots.size(); ++i)
    {
      data += (i > 0) ? "," : "";
      data += snapshots[i].m_timestamp;
    }
    data += "]";
  }

  // Fill the generator data
  for (const auto& supported : node::dataSupported)
  {
    data += ",\"";
    data += supported.m_name;
    data += "\":[";
    // Fill only the current type supported
    for (node::size_t i = 0; i < snapshots.size(); ++i)
    {
      data += (i > 0) ? ",{" : "{";
      bool isFirst = true;
      for (const auto& generator : snapshots[i].m_generators)
      {
        if (generator.m_type == supported.m_type)
        {
          data += (isFirst) ? "\"" : ",\"";
          data += generator.m_name;
          data += "\":";
          data += generator.m_value;
          isFirst = false;
        }
      }
      data += "}";
    }
    data += "]";
  }

  data += "}";
  return data;
}

void waitForResponse(WiFiSSLClient& client, int timeoutS)
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
    node::data::Analog dataPhotoresistor("photoresistor", PIN_A7, node::DataType::LUMINOSITY, [](const node::data::Generator::value_type value) -> node::data::Generator::value_type {
      return 255 - value;
    });
    node::data::Analog dataMoisture("moisture", PIN_A6, node::DataType::MOISTURE, [](const node::data::Generator::value_type value) -> node::data::Generator::value_type {
      const auto newValue = 255 - value;
      if (newValue > 127) {
        return 255;
      }
      return (newValue << 1);
    });

    node::Array<node::data::Generator::ptr_type, 3> dataGenerators(&dataDht, &dataPhotoresistor, &dataMoisture);

    // Allocate a new entry and get its reference
    snapshots.push_back({
      .m_timestamp = 0
    });
    auto& generators = snapshots[snapshots.size() - 1].m_generators;

    for (auto& data : dataGenerators)
    {
      data->start();

      // Gather the data of the generators
      for (const auto& supported : node::dataSupported)
      {
        if (data->isSupportedType(supported.m_type))
        {
          generators.push_back({
            .m_type = supported.m_type,
            .m_value = data->getValue(supported.m_type),
            .m_name = data->getName()
          });
        }
      }

      data->stop();
    }
}

void saveSnapshot(Snapshots& snapshots)
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
    }
    while (!isConnected && nbAttempt < maxAttempt);
    node::error::assertTrue<TopicApp>(isConnected, "Failed to connect to server");
    node::log::info<TopicApp>("Connected to server");
  }

  // Make a HTTP request
  {
    const auto data = toJson(snapshots);
    sendData(client, data);
  }
  waitForResponse(client, 30);

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
  Snapshots snapshots;

  node::system::start();

  delay(1000);

  takeSnapshot(snapshots);
  saveSnapshot(snapshots);

  delay(10000);

  node::system::restartAfter32min();
}
