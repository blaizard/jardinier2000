#include "wifi.h"
#include "error.h"
#include "log.h"

#include <WiFiNINA.h>
#include "utility/wifi_drv.h"

void node::wifi::sleep()
{
	WiFiDrv::wifiDriverDeinit();
	pinMode(NINA_RESETN, OUTPUT);
	digitalWrite(NINA_RESETN, LOW);
}

void node::wifi::wakeUp()
{
	pinMode(NINA_RESETN, OUTPUT);
	digitalWrite(NINA_RESETN, HIGH);
	WiFiDrv::wifiDriverInit();
}

node::wifi::Scope::Scope(const node::string& ssid, const node::string& key)
{
	connect(ssid, key);
}

node::wifi::Scope::~Scope()
{
	disconnect();
}

void node::wifi::Scope::connect(const node::string& ssid, const node::string& key)
{
	node::wifi::wakeUp();

	// Check for the WiFi module:
	node::error::assertTrue<Topic>(WiFi.status() != WL_NO_MODULE, "No module found");

	const String fwVersion = WiFi.firmwareVersion();
	node::error::assertTrue<Topic>(fwVersion != WIFI_FIRMWARE_LATEST_VERSION, "Firmware version is too old: ", fwVersion, ", latest: ", WIFI_FIRMWARE_LATEST_VERSION);

	// Attempt to connect to WiFi network:
	node::log::info<Topic>("Trying to connect to SSID '", ssid, "'...");
	{
		// Connect to WPA/WPA2 network.
		WiFi.begin(ssid.c_str(), key.c_str());

		constexpr uint8_t timeoutS = 20;
		uint8_t timeS = 0;
		do
		{
			// wait 1 second for connection:
			delay(1000);
			++timeS;
		} while (WiFi.status() == WL_IDLE_STATUS && timeS < timeoutS);
		delay(100);

		node::error::assertTrue<Topic>(timeS < timeoutS, "Timeout (", timeS, "s)");
	}

	const auto status = WiFi.status();
	node::error::assertTrue<Topic>(status == WL_CONNECTED, "Failed to connect to SSID '", ssid , "', status=", status);

	node::log::info<Topic>("Connected with ", WiFi.localIP()[0], ".", WiFi.localIP()[1], ".", WiFi.localIP()[2], ".", WiFi.localIP()[3], ", signal strength (RSSI): ", WiFi.RSSI(), "dBm");
}

void node::wifi::Scope::disconnect()
{
	WiFi.disconnect();
	WiFi.end();
	delay(1000);
	const auto status = WiFi.status();
	node::error::assertTrue<Topic>(status == WL_IDLE_STATUS, "Failed to connect, status=", status);
	node::wifi::sleep();
	node::log::info<Topic>("Disconnected");
}
