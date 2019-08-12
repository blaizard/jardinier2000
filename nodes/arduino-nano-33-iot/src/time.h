#pragma once

#include "topic.h"
#include "types.h"

namespace node
{
	namespace time
	{
		using time_type = ::node::uint32_t;
		using delay_type = ::node::uint32_t;

		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "time";
		};

		/**
		 * Delay the execution of the program for a certain amount of time.
		 */
		void delayMs(const delay_type ms);
	}
}
