/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include <vector>
#include <stddef.h>

namespace catapult { namespace utils {

	/// Fixed size circular buffer.
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
		/// Gets the size of the buffer.
		constexpr size_t size() const {
			return m_next < m_capacity ? m_next : m_capacity;
		}

		/// Gets the capacity of the buffer.
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
		inline size_t incrementNext() {
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
