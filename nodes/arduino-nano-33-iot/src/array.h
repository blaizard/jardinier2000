#pragma once

namespace node
{
	template <class T, unsigned int N>
	class Array
	{
	public:

		class Iterator
		{
		public:
			Iterator(Array<T, N>& array, const unsigned int index)
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
			unsigned int m_index;
		};

		class ConstIterator
		{
		public:
			ConstIterator(const Array<T, N>& array, const unsigned int index)
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
			unsigned int m_index;
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

		unsigned int size() const noexcept
		{
			return N;
		}

		unsigned int capacity() const noexcept
		{
			return N;
		}

		T& operator[](const unsigned int index)
		{
			return m_data[index];
		}

		const T& operator[](const unsigned int index) const
		{
			return m_data[index];
		}

	protected:
		T m_data[N];
	};
}
