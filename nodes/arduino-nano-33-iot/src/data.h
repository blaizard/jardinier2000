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
			Generator(const DataType supportedTypes)
					: m_supportedTypes(supportedTypes)
			{
			}
			virtual ~Generator() = default;

			virtual DataType getSupportedTypes() const noexcept
			{
				return m_supportedTypes;
			}

			virtual void start() = 0;
			virtual void stop() = 0;
			virtual uint16_t getValue(const DataType type) = 0;

		private:
			DataType m_supportedTypes;
		};
	}
}
