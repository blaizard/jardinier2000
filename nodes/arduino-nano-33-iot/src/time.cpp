#include "time.h"
#include "persistence.h"

#include <Arduino.h>

node::time::time_type node::time::getTime() noexcept
{
	return persistence::Data.time++;
}

void node::time::delayMs(const delay_type ms)
{
	delay(ms);
}
