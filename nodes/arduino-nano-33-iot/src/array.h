#pragma once

#include "types.h"

namespace node
{
	template <class T, size_t N>
	class Array
	{
	protected:
		using size_type = size_t;

	public:
		class Iterator
		{
		public:
			Iterator(Array<T, N>& array, const size_type index)
				: m_array(array), m_index(index)
			{
			}

			Iterator& operator++() noexcept
			{
				++m_index;
				return *this;
			}

			bool operator==(const Iterator& it) const noexcept
			{
				return it.m_index == m_index;
			}

			bool operator!=(const Iterator& it) const noexcept
			{
				return !(it == *this);
			}

			T& operator*()
			{
				return m_array[m_index];
			}

		private:
			Array<T, N>& m_array;
			size_type m_index;
		};

		class ConstIterator
		{
		public:
			ConstIterator(const Array<T, N>& array, const size_type index)
				: m_array(array), m_index(index)
			{
			}

			ConstIterator& operator++() noexcept
			{
				++m_index;
				return *this;
			}

			bool operator==(const ConstIterator& it) const noexcept
			{
				return it.m_index == m_index;
			}

			bool operator!=(const ConstIterator& it) const noexcept
			{
				return !(it == *this);
			}

			const T& operator*() const
			{
				return m_array[m_index];
			}

		private:
			const Array<T, N>& m_array;
			size_type m_index;
		};

	public:
		using iterator = Iterator;
		using const_iterator = ConstIterator;

		template <class... Args>
		Array(Args&&... args)
				: m_data{args...}
		{
		}

		iterator begin()
		{
			return iterator(*this, 0);
		}

		iterator end()
		{
			return iterator(*this, size());
		}

		const_iterator begin() const
		{
			return const_iterator(*this, 0);
		}

		const_iterator end() const
		{
			return const_iterator(*this, size());
		}

		size_type size() const noexcept
		{
			return N;
		}

		size_type capacity() const noexcept
		{
			return N;
		}

		T& operator[](const size_type index)
		{
			return m_data[index];
		}

		const T& operator[](const size_type index) const
		{
			return m_data[index];
		}

		T* data()
		{
			return m_data;
		}

		const T* data() const
		{
			return m_data;
		}

	protected:
		T m_data[N];
	};
}
