#pragma once

#include "topic.h"
#include "types.h"
#include "array.h"

namespace node
{
	namespace data
	{
		struct Topic : public ::node::Topic
		{
			static constexpr const char* toString = "data";
		};

		enum class Type : uint8_t
		{
			HUMIDITY = 0x01,
			TEMPERATURE = 0x02,
			MOISTURE = 0x04,
			LUMINOSITY = 0x08,
			BATTERY = 0x10
		};
		DEFINE_ENUM_BITWISE_OPERATORS(Type, uint8_t);

		struct Data
		{
			const Type m_type;
			const char* const m_name;
		};
		extern Array<Data, 5> supported;

		class Generator
		{
		public:
			using ptr_type = Generator*;
			using value_type = uint8_t;
			using name_type = const char*;

			Generator(const Type supportedTypes)
					: m_supportedTypes(supportedTypes)
			{
			}
			virtual ~Generator() = default;

			bool isSupportedType(const Type type) const noexcept
			{
				return (static_cast<int>(m_supportedTypes & type) != 0);
			}

			virtual void start() = 0;
			virtual void stop() = 0;
			/**
			 * \todo need to ensure that the string is literal
			 */
			virtual name_type getName() = 0;
			virtual value_type getValue(const Type type) = 0;

		protected:
			Type m_supportedTypes;
		};
	}
}
