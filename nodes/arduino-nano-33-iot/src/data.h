#pragma once

#include "topic.h"
#include "types.h"

namespace node
{
	namespace data
	{
		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "data";
		};

		class Generator
		{
		public:
			using ptr_type = Generator*;
			using value_type = uint8_t;

			Generator(const DataType supportedTypes)
					: m_supportedTypes(supportedTypes)
			{
			}
			virtual ~Generator() = default;

			bool isSupportedType(const DataType type) const noexcept
			{
				return (static_cast<int>(m_supportedTypes & type) != 0);
			}

			virtual void start() = 0;
			virtual void stop() = 0;
			virtual value_type getValue(const DataType type) = 0;

		private:
			DataType m_supportedTypes;
		};
	}
}
