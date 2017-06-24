#pragma once
#include "catapult/preprocessor.h"
#include <vector>
#include <stddef.h>

namespace catapult { namespace utils {

	/// A fixed size circular buffer.
	template<typename T>
	class CircularBuffer {
	public:
		/// Creates a circular buffer with the specified \a size.
		explicit CircularBuffer(size_t size)
				: m_capacity(size)
				, m_buffer(size)
				, m_next(0)
		{}

	public:
		/// Appends \a element to the end of the buffer, possibly overwriting existing elements.
		void push_back(const T& element) {
			m_buffer[incrementNext()] = element;
		}

		/// Appends \a element to the end of the buffer, possibly overwriting existing elements.
		void push_back(T&& element) {
			m_buffer[incrementNext()] = std::move(element);
		}

	public:
		/// The size of the buffer.
		constexpr size_t size() const {
			return m_next < m_capacity ? m_next : m_capacity;
		}

		/// The capacity of the buffer.
		constexpr size_t capacity() const {
			return m_capacity;
		}

	public:
		/// Gets the element at \a index.
		T& operator[](size_t index) {
			return m_buffer[truncateIndex(index)];
		}

		/// Gets the element at \a index.
		const T& operator[](size_t index) const {
			return m_buffer[truncateIndex(index)];
		}

	private:
		CATAPULT_INLINE size_t incrementNext() {
			size_t next = truncateIndex(m_next);
			++m_next;
			return next;
		}

		constexpr size_t truncateIndex(size_t index) const {
			return index % m_capacity;
		}

	private:
		const size_t m_capacity;
		std::vector<T> m_buffer;
		size_t m_next;
	};
}}
