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
#include "KeyPair.h"

namespace catapult { namespace crypto {

	struct SharedKey_tag { static constexpr size_t Size = 32; };
	using SharedKey = utils::ByteArray<SharedKey_tag>;

	/// Generates HKDF of \a sharedSecret using default zeroed salt and constant label "catapult".
	SharedKey Hkdf_Hmac_Sha256_32(const Key& sharedSecret);

	/// Derives shared secret from \a keyPair and \a otherPublicKey.
	Key DeriveSharedSecret(const KeyPair& keyPair, const Key& otherPublicKey);

	/// Generates shared key using \a keyPair and \a otherPublicKey.
	/// \note: One of the provided keys is expected to be an ephemeral key.
	SharedKey DeriveSharedKey(const KeyPair& keyPair, const Key& otherPublicKey);
}}
