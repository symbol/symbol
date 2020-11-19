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
#include "SecureZero.h"
#include "catapult/utils/ByteArray.h"
#include "catapult/utils/HexParser.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/functions.h"

namespace catapult { namespace crypto {

	/// Base class for wrappers of secure byte array types, to provide some type-safety.
	template<typename TTag>
	class SecureByteArray : public utils::MoveOnly {
	public:
		/// Size of the underlying array data.
		static constexpr size_t Size = TTag::Size;

	public:
		/// Creates a byte array.
		SecureByteArray() = default;

		/// Destroys the byte array.
		~SecureByteArray() {
			SecureZero(m_array);
		}

	public:
		/// Move constructor.
		SecureByteArray(SecureByteArray&& rhs) {
			SecureZeroGuard guard(rhs.m_array);
			m_array = std::move(rhs.m_array);
		}

		/// Move assignment operator.
		SecureByteArray& operator=(SecureByteArray&& rhs) {
			SecureZeroGuard guard(rhs.m_array);
			m_array = std::move(rhs.m_array);
			return *this;
		}

	public:
		/// Gets the array size.
		constexpr size_t size() const {
			return m_array.size();
		}

		/// Gets a const pointer to the underlying array.
		constexpr const uint8_t* data() const noexcept {
			return m_array.data();
		}

		/// Gets a const iterator to the first byte.
		constexpr auto begin() const noexcept {
			return m_array.begin();
		}

		/// Gets a const iterator to one past the last byte.
		constexpr auto end() const noexcept {
			return m_array.end();
		}

	public:
		/// Returns \c true if this value is equal to \a rhs.
		constexpr bool operator==(const SecureByteArray& rhs) const {
			return m_array == rhs.m_array;
		}

		/// Returns \c true if this value is not equal to \a rhs.
		constexpr bool operator!=(const SecureByteArray& rhs) const {
			return !(*this == rhs);
		}

	public:
		/// Creates a byte array from \a buffer.
		static SecureByteArray FromBuffer(const RawBuffer& buffer) {
			if (Size != buffer.Size)
				CATAPULT_THROW_INVALID_ARGUMENT_1("secure byte array input buffer has unexpected size", buffer.Size);

			SecureByteArray byteArray;
			std::memcpy(byteArray.m_array.data(), buffer.pData, buffer.Size);
			return byteArray;
		}

		/// Creates a byte array from \a buffer and securely erases it.
		static SecureByteArray FromBufferSecure(const MutableRawBuffer& buffer) {
			SecureZeroGuard guard(buffer.pData, buffer.Size);
			return FromBuffer(RawBuffer{ buffer.pData, buffer.Size });
		}

		/// Creates a byte array from \a str.
		static SecureByteArray FromString(const RawString& str) {
			SecureByteArray byteArray;
			utils::ParseHexStringIntoContainer(str.pData, str.Size, byteArray.m_array);
			return byteArray;
		}

		/// Creates a byte array from \a str and securely erases it.
		static SecureByteArray FromStringSecure(const MutableRawString& str) {
			SecureZeroGuard guard(reinterpret_cast<uint8_t*>(str.pData), str.Size);
			return FromString(RawString{ str.pData, str.Size });
		}

		/// Generates a new byte array using the specified byte \a generator.
		static SecureByteArray Generate(const supplier<uint8_t>& generator) {
			SecureByteArray byteArray;
			std::generate_n(byteArray.m_array.begin(), byteArray.m_array.size(), generator);
			return byteArray;
		}

	private:
		utils::ByteArray<TTag> m_array;

	private:
		class SecureZeroGuard {
		public:
			explicit SecureZeroGuard(utils::ByteArray<TTag>& byteArray) : SecureZeroGuard(byteArray.data(), byteArray.size())
			{}

			SecureZeroGuard(uint8_t* pData, size_t dataSize) : m_pData(pData), m_dataSize(dataSize)
			{}

			~SecureZeroGuard() {
				SecureZero(m_pData, m_dataSize);
			}

		private:
			uint8_t* m_pData;
			size_t m_dataSize;
		};
	};
}}
