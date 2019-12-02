#pragma once

#include "topic.h"
#include "types.h"

namespace node
{
	namespace wifi
	{
		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "wifi";
		};

		void sleep();
		void wakeUp();

		class Scope
		{
		public:
			explicit Scope(const node::string& ssid, const node::string& key);

			~Scope();

		private:
			/**
			 * Connect to the specified network
			 */
			void connect(const node::string& ssid, const node::string& key);
			void disconnect();
		};
	}
}
