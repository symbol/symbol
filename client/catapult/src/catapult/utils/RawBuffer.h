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
#include "traits/Traits.h"
#include <ostream>
#include <string>
#include <stddef.h>
#include <stdint.h>

namespace catapult { namespace utils {

	/// Basic raw buffer that is composed of a pointer and a size.
	template<typename T>
	class BasicRawBuffer {
	public:
		/// Creates an empty buffer.
		constexpr BasicRawBuffer() : BasicRawBuffer(nullptr, 0)
		{}

		/// Creates a buffer around the entire contents of \a container.
		template<
				typename TContainer,
				// disable when copy/move constructors should be used
				typename X = std::enable_if_t<!traits::is_base_of_ignore_reference_v<BasicRawBuffer, TContainer>>,
				// disable when other constructors are better match
				typename Y = std::enable_if_t<!std::is_scalar_v<TContainer>>
		>
		BasicRawBuffer(TContainer&& container) : BasicRawBuffer(container.data(), container.size())
		{}

		/// Creates buffer around \a pRawBuffer pointer and \a size.
		constexpr BasicRawBuffer(T* pRawBuffer, size_t size) : pData(pRawBuffer), Size(size)
		{}

	public:
		/// Data pointer.
		T* pData;

		/// Data size.
		size_t Size;
	};

	/// Const binary buffer.
	using RawBuffer = BasicRawBuffer<const uint8_t>;

	/// Mutable binary buffer.
	using MutableRawBuffer = BasicRawBuffer<uint8_t>;

	/// Const string buffer.
	class RawString : public BasicRawBuffer<const char> {
	public:
		using BasicRawBuffer<const char>::BasicRawBuffer;

	public:
		/// Creates an empty string buffer.
		constexpr RawString() : BasicRawBuffer()
		{}

		/// Creates a string buffer around a NUL-terminated string (\a str).
		RawString(const char* str);
	};

	/// Mutable string buffer.
	class MutableRawString : public BasicRawBuffer<char> {
	public:
		using BasicRawBuffer<char>::BasicRawBuffer;

	public:
		/// Creates an empty mutable string buffer.
		constexpr MutableRawString() : BasicRawBuffer()
		{}

		/// Creates a mutable string buffer around \a str.
		MutableRawString(std::string& str);
	};

	/// Insertion operator for outputting \a str to \a out.
	template<
			typename T,
			typename X = std::enable_if_t<std::is_same_v<char, typename std::remove_const_t<T>>>>
	std::ostream& operator<<(std::ostream& out, const BasicRawBuffer<T>& str) {
		out.write(str.pData, static_cast<std::streamsize>(str.Size));
		return out;
	}
}}
