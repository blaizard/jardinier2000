#pragma once

#include "types.h"
#include "array.h"

namespace node
{
	template <class T, size_t N>
	class Vector : public Array<T, N>
	{
	protected:
		using Array<T, N>::m_data;
		using typename Array<T, N>::iterator;
		using typename Array<T, N>::const_iterator;
		using typename Array<T, N>::size_type;

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

		size_type size() const noexcept
		{
			return m_size;
		}

		bool empty() const noexcept
		{
			return (m_size == 0);
		}

		void clear() noexcept
		{
			resize(0);
		}

		void resize(const size_t n) noexcept
		{
			m_size = min(n, N);
		}

	protected:
		size_type m_size;
	};
}
