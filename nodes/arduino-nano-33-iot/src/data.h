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
			using name_type = const char*;

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
			/**
			 * \todo need to ensure that the string is literal
			 */
			virtual name_type getName() = 0;
			virtual value_type getValue(const DataType type) = 0;

		protected:
			DataType m_supportedTypes;
		};
	}
}
