#pragma once

#include "topic.h"
#include "types.h"
#include "log.h"

void TC5_Handler();
void WDT_Handler();

namespace node
{
	namespace system
	{
		using delay_type = ::node::uint32_t;
		using timestamp_type = ::node::uint32_t;

		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "system";
		};

		// Current running timestamp available after start has been called
		extern timestamp_type timestamp;

		void start();
		void stop();

		void sleep();
		void sleepFor16s();
		void sleepFor1min();
		void sleepFor8min();
		void sleepFor16min();
		void sleepFor32min();

		void restart();
	}
}
