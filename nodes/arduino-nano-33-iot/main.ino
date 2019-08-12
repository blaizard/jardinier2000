//#include <ArduinoLowPower.h>
#include <Arduino.h>

#include "include/arduino_secrets.h"
#include "src/log.h"
#include "src/error.h"
#include "src/wifi.h"
#include "src/system.h"
#include "src/topic.h"
#include "src/array.h"

#include "src/data/dht.h"

#include <WiFiNINA.h>

// If you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
char server[] = "blaizard.com";    // name address for Google (using DNS)

struct TopicApp : public ::node::Topic
{
	static constexpr const char* toString = "app";
};

struct TopicHttp : public ::node::Topic
{
	static constexpr const char* toString = "http";
};

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiSSLClient client;

void sendData(const int temperature, const int humidity)
{
  String data = "{\"list\":[{\"temperature\":";
  data += temperature;
  data += ",\"humidity\":";
  data += humidity;
  data += "}]}";

  {
    String line = "POST /jardinier2000/api/v1/sample?token=";
    line += NODE_TOKEN;
    line += " HTTP/1.1";
    client.println(line);
  }
  client.println("Cookie: irpath=/jardinier2000");
  client.println("Host: www.blaizard.com");
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

void waitForResponse(int timeoutS)
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

void setup()
{
  node::system::start();

  // Connect to the wifi
  {
    // Setup the sensors
    node::data::DHT dataDht(PIN_A3);

    node::Array<node::data::Generator::ptr_type, 1> dataGenerators(&dataDht);

    node::wifi::Scope scope(SECRET_SSID, SECRET_PASS);

    // Connect to server
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
    node::error::assertTrue<TopicHttp>(isConnected, "Failed to connect to server");

    // Read sensors
    node::log::info<TopicApp>("Connected to server");

    node::data::Generator::value_type temperature;
    node::data::Generator::value_type humidity;    

    for (auto& data : dataGenerators)
    {
      data->start();

      if (data->isSupportedType(node::DataType::TEMPERATURE))
      {
        temperature = data->getValue(node::DataType::TEMPERATURE);
      }
      if (data->isSupportedType(node::DataType::HUMIDITY))
      {
        humidity = data->getValue(node::DataType::HUMIDITY);
      }

      data->stop();
    }

    node::log::info<TopicApp>("Temperature=", temperature, ", Humidity=", humidity);

    // Make a HTTP request
    sendData(temperature, humidity);
    waitForResponse(30);

    delay(1000);

    if (!client.connected())
    {
      node::log::info<TopicApp>("Disconnecting from server");
      client.stop();
    }
  }

  node::system::restartAfter32min();
}

void loop()
{
}
