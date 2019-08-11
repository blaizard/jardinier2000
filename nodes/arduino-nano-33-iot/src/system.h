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

		void start();
		void stop();

		void restart();
		void restartAfter16s();
		void restartAfter1min();
		void restartAfter8min();
	}
}
