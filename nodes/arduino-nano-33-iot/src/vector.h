#pragma once

#include "types.h"
#include "array.h"

namespace node
{
	template <class T, size_t N>
	class Vector : public Array<T, N>
	{
		using Array<T, N>::m_data;
		using typename Array<T, N>::iterator;
		using typename Array<T, N>::const_iterator;

	public:
		template <class... Args>
		Vector(Args&&... args)
				: Array<T, N>{args...}
				, m_size(sizeof...(Args))
		{
		}

		void push_back(const T& element)
		{
			if (m_size < N)
			{
				m_data[m_size] = element;
				++m_size;
			}
		}

		iterator end()
		{
			return iterator(*this, size());
		}

		const_iterator end() const
		{
			return const_iterator(*this, size());
		}

		size_t size() const noexcept
		{
			return m_size;
		}

		void clear() noexcept
		{
			m_size = 0;
		}

	private:
		size_t m_size;
	};
}
