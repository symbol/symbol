/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "HexFormatter.h"
#include <array>

namespace catapult { namespace utils {

	/// Base class for wrappers of byte array types, to provide some type-safety.
	template<size_t N, typename TTag>
	class ByteArray : TTag {
	public:
		using const_iterator = typename std::array<uint8_t, N>::const_iterator;

	public:
		/// Creates a zero-initialized byte array.
		constexpr ByteArray() : m_array()
		{}

		/// Creates a byte array around \a array.
		constexpr ByteArray(const std::array<uint8_t, N>& array) : m_array(array)
		{}

		/// Creates a copy of \a rhs.
		constexpr ByteArray(const ByteArray& rhs) : m_array(rhs.m_array)
		{}

	public:
		/// Assigns \a rhs to this.
		ByteArray& operator=(const ByteArray& rhs) {
			m_array = rhs.m_array;
			return *this;
		}

	public:
		/// Returns the array size.
		constexpr size_t size() const {
			return m_array.size();
		}

		/// Returns a const reference to the byte at \a index.
		constexpr const uint8_t& operator[](size_t index) const {
			return m_array[index];
		}

		/// Returns a reference to the byte at \a index.
		constexpr uint8_t& operator[](size_t index) {
			return m_array[index];
		}

		/// Returns a const pointer to the underlying array.
		constexpr const uint8_t* data() const noexcept {
			return m_array.data();
		}

		/// Returns a pointer to the underlying array.
		constexpr uint8_t* data() noexcept {
			return m_array.data();
		}

	public:
		/// Returns a const iterator to the first byte.
		constexpr auto cbegin() const noexcept {
			return m_array.cbegin();
		}

		/// Returns a const iterator to one past the last byte.
		constexpr auto cend() const noexcept {
			return m_array.cend();
		}

		/// Returns a const iterator to the first byte.
		constexpr auto begin() const noexcept {
			return m_array.begin();
		}

		/// Returns a const iterator to one past the last byte.
		constexpr auto end() const noexcept {
			return m_array.end();
		}

		/// Returns an iterator to the first byte.
		constexpr auto begin() noexcept {
			return m_array.begin();
		}

		/// Returns an iterator to one past the last byte.
		constexpr auto end() noexcept {
			return m_array.end();
		}

	public:
		/// Returns \c true if this value is equal to \a rhs.
		constexpr bool operator==(const ByteArray& rhs) const {
			return m_array == rhs.m_array;
		}

		/// Returns \c true if this value is not equal to \a rhs.
		constexpr bool operator!=(const ByteArray& rhs) const {
			return !(*this == rhs);
		}

		/// Returns \c true if this value is greater than or equal to \a rhs.
		constexpr bool operator>=(const ByteArray& rhs) const {
			return m_array >= rhs.m_array;
		}

		/// Returns \c true if this value is greater than \a rhs.
		constexpr bool operator>(const ByteArray& rhs) const {
			return m_array > rhs.m_array;
		}

		/// Returns \c true if this value is less than or equal to \a rhs.
		constexpr bool operator<=(const ByteArray& rhs) const {
			return m_array <= rhs.m_array;
		}

		/// Returns \c true if this value is less than \a rhs.
		constexpr bool operator<(const ByteArray& rhs) const {
			return m_array < rhs.m_array;
		}

		/// Insertion operator for outputting \a byteArray to \a out.
		friend std::ostream& operator<<(std::ostream& out, const ByteArray& byteArray) {
			out << HexFormat(byteArray.m_array);
			return out;
		}

	private:
		std::array<uint8_t, N> m_array;
	};

	// force compilation error if HexFormat is used with ByteArray
	template<size_t N, typename TTag>
	constexpr void HexFormat(const ByteArray<N, TTag>&);
}}
