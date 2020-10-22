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
#include "VotingKeyPair.h"
#include "catapult/types.h"

namespace catapult { namespace crypto {

#pragma pack(push, 1)

	/// Two-layer Bellare-Miner signature.
	struct BmTreeSignature {
	public:
		/// Parent public key and signature pair.
		struct ParentPublicKeySignaturePair {
			/// Public key.
			VotingKey ParentPublicKey;

			/// Signature.
			VotingSignature Signature;
		};

	public:
		/// Root pair.
		ParentPublicKeySignaturePair Root;

		/// Bottom pair.
		ParentPublicKeySignaturePair Bottom;

	public:
		/// Returns \c true if this signature is equal to \a rhs.
		bool operator==(const BmTreeSignature& rhs) const;

		/// Returns \c true if this signature is not equal to \a rhs.
		bool operator!=(const BmTreeSignature& rhs) const;
	};

	/// Three-layer Bellare-Miner signature.
	struct BmTreeSignatureV1 {
	private:
		struct VotingKey_tag { static constexpr size_t Size = 48; };
		struct VotingSignature_tag { static constexpr size_t Size = 96; };

	public:
		/// Parent public key and signature pair.
		struct ParentPublicKeySignaturePair {
			/// Public key.
			utils::ByteArray<VotingKey_tag> ParentPublicKey;

			/// Signature.
			utils::ByteArray<VotingSignature_tag> Signature;
		};

	public:
		/// Root pair.
		ParentPublicKeySignaturePair Root;

		/// Top pair.
		ParentPublicKeySignaturePair Top;

		/// Bottom pair.
		ParentPublicKeySignaturePair Bottom;

	public:
		/// Returns \c true if this signature is equal to \a rhs.
		bool operator==(const BmTreeSignatureV1& rhs) const;

		/// Returns \c true if this signature is not equal to \a rhs.
		bool operator!=(const BmTreeSignatureV1& rhs) const;
	};

#pragma pack(pop)
}}
