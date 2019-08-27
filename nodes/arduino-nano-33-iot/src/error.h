#pragma once

#include "log.h"
#include "system.h"
#include "time.h"

namespace node
{
	namespace error
	{
		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "error";
		};

		/**
		 * Ensure that the condition validate to true
		 */
		template<class Topic, class... Args>
		bool assertTrue(const bool condition, Args&&... args)
		{
			if (!condition)
			{
				log::fatal<Topic>(args...);
				return false;
			}
			return true;
		}
	}
}
