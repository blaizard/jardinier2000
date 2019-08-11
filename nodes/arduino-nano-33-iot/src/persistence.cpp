#include "persistence.h"
#include "error.h"

#include <Arduino.h>
#include <FlashAsEEPROM.h>

node::persistence::Data::Data()
	: time(0)
	, nbMeasurements(0)
{
	read();
}

void node::persistence::Data::read()
{
	if (EEPROM.isValid())
	{
		for (uint8_t i = 0; i < sizeof(node::persistence::Data); ++i)
		{
			reinterpret_cast<uint8_t*>(this)[i] = EEPROM.read(i);
		}
	}
}

void node::persistence::Data::write()
{
	for (uint8_t i = 0; i < sizeof(node::persistence::Data); ++i)
	{
		EEPROM.write(i, reinterpret_cast<const uint8_t*>(this)[i]);
	}
	EEPROM.commit();
    error::assertTrue<Topic>(EEPROM.isValid(), "Cannot write to EEPROM");
}
