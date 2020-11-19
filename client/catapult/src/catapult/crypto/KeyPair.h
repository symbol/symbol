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
#include "BasicKeyPair.h"
#include "SecureByteArray.h"

namespace catapult { namespace crypto {

	struct PrivateKey_tag { static constexpr size_t Size = 32; };
	using PrivateKey = SecureByteArray<PrivateKey_tag>;

	/// ED25519 key pair traits.
	struct Ed25519KeyPairTraits {
	public:
		using PublicKey = Key;
		using PrivateKey = crypto::PrivateKey;

	public:
		/// Extracts a public key (\a publicKey) from a private key (\a privateKey).
		static void ExtractPublicKeyFromPrivateKey(const PrivateKey& privateKey, PublicKey& publicKey);
	};

	/// ED25519 key pair.
	/// \note This type does not have a prefix because it's the default signature scheme used in catapult.
	using KeyPair = BasicKeyPair<Ed25519KeyPairTraits>;

	/// ED25519 utils.
	struct Ed25519Utils {
		/// Formats a private \a key for printing.
		static utils::ContainerHexFormatter<Key::const_iterator> FormatPrivateKey(const PrivateKey& key);

		/// Returns \c true if \a str represents a valid private key, \c false otherwise.
		static bool IsValidPrivateKeyString(const std::string& str);
	};
}}
