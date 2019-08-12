#pragma once

#include <Arduino.h>

/**
 * \brief Declare and defines all bitwise operators for enum classes.
 */
#define DEFINE_ENUM_BITWISE_OPERATORS(enumClass, type) \
	inline enumClass operator|(const enumClass& lhs, const enumClass& rhs) noexcept \
	{ \
		return static_cast<enumClass>(static_cast<type>(lhs) | static_cast<type>(rhs)); \
	} \
	inline enumClass operator&(const enumClass& lhs, const enumClass& rhs) noexcept \
	{ \
		return static_cast<enumClass>(static_cast<type>(lhs) & static_cast<type>(rhs)); \
	} \
	inline enumClass operator^(const enumClass& lhs, const enumClass& rhs) noexcept \
	{ \
		return static_cast<enumClass>(static_cast<type>(lhs) ^ static_cast<type>(rhs)); \
	} \
	inline enumClass operator~(const enumClass& rhs) noexcept \
	{ \
		return static_cast<enumClass>(~static_cast<type>(rhs)); \
	} \
	inline enumClass operator|=(enumClass& lhs, const enumClass& rhs) noexcept \
	{ \
		lhs = static_cast<enumClass>(static_cast<type>(lhs) | static_cast<type>(rhs)); \
		return lhs; \
	} \
	inline enumClass operator&=(enumClass& lhs, const enumClass& rhs) noexcept \
	{ \
		lhs = static_cast<enumClass>(static_cast<type>(lhs) & static_cast<type>(rhs)); \
		return lhs; \
	} \
	inline enumClass operator^=(enumClass& lhs, const enumClass& rhs) noexcept \
	{ \
		lhs = static_cast<enumClass>(static_cast<type>(lhs) ^ static_cast<type>(rhs)); \
		return lhs; \
	}

namespace node
{
	using uint8_t = uint8_t;
	using uint16_t = uint16_t;
	using uint32_t = uint32_t;
	using uint64_t = uint64_t;
	using int8_t = int8_t;
	using int16_t = int16_t;
	using int32_t = int32_t;
	using int64_t = int64_t;
	using string = String;
	using size_t = uint32_t;

	using PinType = uint8_t;

	enum class DataType : uint8_t
	{
		HUMIDITY = 0x01,
		TEMPERATURE = 0x02,
		MOISTURE = 0x04,
		LUMINOSITY = 0x08
	};
	DEFINE_ENUM_BITWISE_OPERATORS(DataType, uint8_t);
}
