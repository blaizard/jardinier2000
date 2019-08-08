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

		namespace internal
		{
			/**
			 * Raise a fatal error
			 */
			void fatal();
		}

		/**
		 * Ensure that the condition validate to true
		 */
		template<class Topic, class... Args>
		void assertTrue(const bool condition, Args&&... args)
		{
			if (!condition)
			{
				log::fatal<Topic>(args...);
				internal::fatal();
			}
		}
	}
}
