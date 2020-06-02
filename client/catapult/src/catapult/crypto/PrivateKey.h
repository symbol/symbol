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
#include "catapult/functions.h"
#include "catapult/types.h"

namespace catapult { namespace crypto {

#ifdef SPAMMER_TOOL
#pragma pack(push, 16)
#endif

	/// Represents a private key.
	class PrivateKey final {
	public:
		/// Size of the underlying key data.
		static constexpr size_t Size = Key::Size;

	public:
		/// Creates a private key.
		PrivateKey() = default;

		/// Destroys the private key.
		~PrivateKey();

	public:
		/// Move constructor.
		PrivateKey(PrivateKey&& rhs);

		/// Move assignment operator.
		PrivateKey& operator=(PrivateKey&& rhs);

	public:
		/// Creates a private key from \a str.
		static PrivateKey FromString(const std::string& str);

		/// Creates a private key from \a pRawKey with size \a keySize and securely erases \a pRawKey.
		static PrivateKey FromStringSecure(char* pRawKey, size_t keySize);

		/// Generates a new private key using the specified byte \a generator.
		static PrivateKey Generate(const supplier<uint8_t>& generator);

	public:
		/// Gets a const iterator to the beginning of the raw key.
		inline auto begin() const {
			return m_key.cbegin();
		}

		/// Gets a const iterator to the end of the raw key.
		inline auto end() const {
			return m_key.cend();
		}

		/// Gets the size of the key.
		inline auto size() const {
			return m_key.size();
		}

		/// Gets a const pointer to the raw key.
		inline auto data() const {
			return m_key.data();
		}

	public:
		/// Returns \c true if this key and \a rhs are equal.
		bool operator==(const PrivateKey& rhs) const;

		/// Returns \c true if this key and \a rhs are not equal.
		bool operator!=(const PrivateKey& rhs) const;

	private:
		static PrivateKey FromString(const char* const pRawKey, size_t keySize);

	private:
		Key m_key;
	};

#ifdef SPAMMER_TOOL
#pragma pack(pop)
#endif
}}
