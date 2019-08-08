#pragma once

#include "topic.h"
#include "types.h"
#include "log.h"

namespace node
{
	namespace system
	{
		using delay_type = ::node::uint32_t;

		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "system";
		};

		void reboot();
		void sleep();

		void rebootAfter16s();
		void rebootAfter1min();
		void rebootAfter8min();
	}
}
