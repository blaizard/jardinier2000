#pragma once

#include "topic.h"
#include "time.h"
#include "types.h"

namespace node
{
	namespace persistence
	{
		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "persistence";
		};

		/**
		 * Data being persisted
		 */
		static struct Data
		{
			Data();

			void read();
			void write();

			time::time_type time;
			uint8_t nbMeasurements;
			struct
			{
				::node::DataType type;
				uint8_t value;
			} measurements[20];
		} Data;
	}
}
