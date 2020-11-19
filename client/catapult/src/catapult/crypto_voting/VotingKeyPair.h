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
#include "catapult/crypto/BasicKeyPair.h"
#include "catapult/crypto/SecureByteArray.h"

namespace catapult { namespace crypto {

	struct VotingPrivateKey_tag { static constexpr size_t Size = 32; };
	using VotingPrivateKey = SecureByteArray<VotingPrivateKey_tag>;

	struct VotingSignature_tag { static constexpr size_t Size = 64; };
	using VotingSignature = utils::ByteArray<VotingSignature_tag>;

	/// BLS 381-12 key pair traits.
	struct VotingKeyPairTraits {
	public:
		using PublicKey = VotingKey;
		using PrivateKey = VotingPrivateKey;

	public:
		/// Extracts a public key (\a publicKey) from a private key (\a privateKey).
		static void ExtractPublicKeyFromPrivateKey(const PrivateKey& privateKey, PublicKey& publicKey);
	};

	/// BLS 381-12 key pair.
	/// \note This key pair is used for voting messages.
	using VotingKeyPair = BasicKeyPair<VotingKeyPairTraits>;
}}
