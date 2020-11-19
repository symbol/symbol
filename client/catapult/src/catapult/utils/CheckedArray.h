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
#include "catapult/exceptions.h"

namespace catapult { namespace utils {

	/// Array that checks for overflow.
	template<typename T, size_t N>
	class CheckedArray {
	public:
		CheckedArray() : m_size(0)
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
		/// Gets a const iterator that represents first element.
		const T* begin() const {
			return &m_data[0];
		}

		/// Gets a const iterator that represents one past last element.
		const T* end() const {
			return &m_data[m_size];
		}

		/// Gets a const iterator that represents first element.
		const T* cbegin() const {
			return &m_data[0];
		}

		/// Gets a const iterator that represents one past last element.
		const T* cend() const {
			return &m_data[m_size];
		}

	public:
		/// Gets a reference to the element at \a index.
		/// \note This method throws if the index is out of range.
		T& operator[](size_t index) {
			if (index >= m_size)
				CATAPULT_THROW_OUT_OF_RANGE("index out of range");

			return m_data[index];
		}

		/// Gets the element at \a index.
		/// \note This method throws if the index is out of range.
		const T& operator[](size_t index) const {
			if (index >= m_size)
				CATAPULT_THROW_OUT_OF_RANGE("index out of range");

			return m_data[index];
		}

		/// Returns \c true if this array is equal to \a rhs.
		constexpr bool operator==(const CheckedArray& rhs) const {
			return m_size == rhs.m_size && 0 == std::memcmp(m_data, rhs.m_data, m_size * sizeof(T));
		}

		/// Returns \c true if this array is not equal to \a rhs.
		constexpr bool operator!=(const CheckedArray& rhs) const {
			return !(*this == rhs);
		}

	private:
		T m_data[N];
		size_t m_size;
	};
}}
