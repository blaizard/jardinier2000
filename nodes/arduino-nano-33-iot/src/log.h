#pragma once

#include "types.h"
#include "time.h"

namespace node
{
	namespace log
	{
		namespace internal
		{
			template<class T, class... Args>
			inline void buildMessage(node::string& message, T&& str, Args&&... args)
			{
				message += str;
				buildMessage(message, args...);
			}

			/**
			 * Log a message
			 */
			template<class Topic, class... Args>
			void message(const char* const pre, Args&&... args)
			{
				if (Serial)
				{
					node::string message;
					message += "[";
					message += time::getTime();
					message += "] ";
					message += pre;
					message += "[";
					message += Topic::toString;
					message += "] ";
					buildMessage(message, args...);
					Serial.print(message + "\n");
				}
			}
		}

		template<class Topic, class... Args>
		void info(Args&&... args)
		{
			internal::message<Topic>("[info] ", args...);
		}

		template<class Topic, class... Args>
		void warning(Args&&... args)
		{
			internal::message<Topic>("[warning] ", args...);
		}

		template<class Topic, class... Args>
		void error(Args&&... args)
		{
			internal::message<Topic>("[error] ", args...);
		}

		template<class Topic, class... Args>
		void fatal(Args&&... args)
		{
			internal::message<Topic>("[fatal] ", args...);
		}
	}
}

// Partial template specialization, it has to be done outside of the namespac
template<class T>
inline void ::node::log::internal::buildMessage(node::string& message, T&& str)
{
	message += str;
}
