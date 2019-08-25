#pragma once

#include "types.h"

namespace node
{
	template <class T, size_t N>
	class Array
	{
	public:

		class Iterator
		{
		public:
			Iterator(Array<T, N>& array, const size_t index)
				: m_array(array), m_index(index)
			{
			}

			Iterator& operator++()
			{
				++m_index;
				return *this;
			}

			bool operator==(Iterator& it)
			{
				return it.m_index == m_index;
			}

			bool operator!=(Iterator& it)
			{
				return !(it == *this);
			}

			T& operator*()
			{
				return m_array[m_index];
			}

		private:
			Array<T, N>& m_array;
			size_t m_index;
		};

	public:
		using iterator = Iterator;

		template <class... Args>
		Array(Args&&... args)
				: m_data{args...}
		{
		}

		iterator begin()
		{
			return Iterator(*this, 0);
		}

		iterator end()
		{
			return Iterator(*this, size());
		}

		size_t size() const noexcept
		{
			return N;
		}

		size_t capacity() const noexcept
		{
			return N;
		}

		T& operator[](const size_t index)
		{
			return m_data[index];
		}

	protected:
		T m_data[N];
	};
}
