//#include <ArduinoLowPower.h>
#include <Arduino.h>

#include "include/arduino_secrets.h"
#include "src/log.h"
#include "src/error.h"
#include "src/wifi.h"
#include "src/system.h"
#include "src/topic.h"

#include "src/DHTLib/dht.h"

#include <SPI.h>
#include <WiFiNINA.h>

char token[] = NODE_TOKEN;

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

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
/*  while (!Serial)
  {
    delay(1000); // Needed for native USB port only
  }
*/
  // Connect to the wifi
  {
    node::wifi::Scope scope(SECRET_SSID, SECRET_PASS);

    dht DHT;
    DHT.read11(PIN_A3);

    node::log::info<node::wifi::Topic>("Temperature=", DHT.temperature, ", Humidity=", DHT.humidity);

    Serial.println("\nStarting connection to server...");
    // if you get a connection, report back via serial:
    const bool isConnected = client.connect(server, 443);
    node::error::assertTrue<TopicHttp>(isConnected, "Failed to connect to server");

    Serial.println("connected to server");

    // Make a HTTP request
    sendData(DHT.temperature, DHT.humidity);

    delay(1000);

    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available())
    {
      char c = client.read();
      Serial.write(c);
    }

    delay(1000);

    if (!client.connected())
    {
      Serial.println();
      Serial.println("disconnecting from server.");
      client.stop();
    }
  }

  node::log::info<node::wifi::Topic>("Rebooting in 8min...");
  node::system::rebootAfter8min();
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}
