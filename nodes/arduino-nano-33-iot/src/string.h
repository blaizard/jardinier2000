#pragma once

#include "types.h"
#include "vector.h"

namespace node
{
	template <size_t N>
	class String : public Vector<char, N + 1>
	{
	protected:
		using Vector<char, N + 1>::m_data;
		using Vector<char, N + 1>::m_size;
		using typename Vector<char, N + 1>::size_type;

	public:
		String()
		{
			resize(0);
		}

		// Remove vector specific functions
		void push_back(const char& element) = delete;

		// Update some functions
		void resize(const size_t n) noexcept
		{
			m_size = min(n, N);
			m_data[m_size] = '\0';
		}

		size_type capacity() const noexcept
		{
			return N;
		}

		// Converter
		void append(const char* str) noexcept
		{
			append(str, strlen(str));
		}

		void append(const char c) noexcept
		{
			append(&c, 1);
		}

		void append(const uint64_t n) noexcept
		{
			appendUnsignedInt<uint64_t>(n);
		}
		void append(const uint32_t n) noexcept
		{
			appendUnsignedInt<uint32_t>(n);
		}
		void append(const uint16_t n) noexcept
		{
			appendUnsignedInt<uint32_t>(n);
		}
		void append(const uint8_t n) noexcept
		{
			appendUnsignedInt<uint8_t>(n);
		}

		void append(const int64_t n) noexcept
		{
			appendSignedInt<int64_t>(n);
		}
		void append(const int32_t n) noexcept
		{
			appendSignedInt<int32_t>(n);
		}
		void append(const int16_t n) noexcept
		{
			appendSignedInt<int16_t>(n);
		}
		void append(const int8_t n) noexcept
		{
			appendSignedInt<int8_t>(n);
		}

		void append(const char* str, const size_type n) noexcept
		{
			// Handles overflows
			const size_type actualN = min(N - m_size, n);
			memcpy(&m_data[m_size], str, actualN);
			m_size += actualN;
			m_data[m_size] = '\0';
		}

		template <class T>
		String& operator=(const T data) noexcept
		{
			resize(0);
			append(data);
			return *this;
		}

		template <class T>
		String& operator+=(const T data) noexcept
		{
			append(data);
			return *this;
		}

	private:
		template <class T>
		void appendSignedInt(const T n) noexcept
		{
			if (n < 0)
			{
				append('-');
				appendUnsignedInt(-n);
			}
			else
			{
				appendUnsignedInt(n);
			}
		}

		template <class T>
		void appendUnsignedInt(const T n) noexcept
		{
			char* pBuffer = &m_data[m_size];
			const size_type bufferSize = N - m_size;
			if (bufferSize)
			{
				char* pStr = &pBuffer[bufferSize];
				T number = n;

				do
				{
					const int digit = static_cast<int>(number % 10) + '0';
					number /= 10;
					*--pStr = static_cast<char>(digit);
				} while (number && pBuffer < pStr);

				const size_type size = static_cast<size_type>(&pBuffer[bufferSize] - pStr);
				memmove(pBuffer, pStr, size);
				m_size += size;
				m_data[m_size] = '\0';
			}
		}
	};
}
