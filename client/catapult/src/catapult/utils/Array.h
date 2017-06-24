#pragma once
#include "catapult/exceptions.h"

namespace catapult { namespace utils {

	/// An array that checks for overflow.
	template<typename T, size_t N>
	class Array {
	public:
		Array() : m_size(0)
		{}

	public:
		/// Returns \c true if the array is empty, \c false otherwise.
		bool empty() const {
			return 0 == m_size;
		}

		/// Gets the size of the array.
		size_t size() const {
			return m_size;
		}

		/// Gets the capacity of the array.
		size_t capacity() const {
			return N;
		}

		/// Appends \a val at the end of the array.
		/// \note This method throws if the boundary of the array is exceeded.
		void push_back(T val) {
			if (N <= m_size)
				CATAPULT_THROW_OUT_OF_RANGE("array bounds exeeded");

			m_data[m_size++] = val;
		}

	public:
		/// Returns a reference to the element at \a index.
		/// \note This method throws if the index is out of range.
		T& operator[](size_t index) {
			if (index >= m_size)
				CATAPULT_THROW_OUT_OF_RANGE("index out of range");

			return m_data[index];
		}

		/// Returns the element at \a index.
		/// \note This method throws if the index is out of range.
		const T& operator[](size_t index) const {
			if (index >= m_size)
				CATAPULT_THROW_OUT_OF_RANGE("index out of range");

			return m_data[index];
		}

		/// Returns \c true if this array is equal to \a rhs.
		constexpr bool operator==(const Array& rhs) const {
			return m_size == rhs.m_size && 0 == std::memcmp(m_data, rhs.m_data, m_size * sizeof(T));
		}

		/// Returns \c true if this array is not equal to \a rhs.
		constexpr bool operator!=(const Array& rhs) const {
			return !(*this == rhs);
		}

	private:
		T m_data[N];
		size_t m_size;
	};
}}
