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
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Cosignature attached to an aggregate transaction.
	struct Cosignature {
	public:
		/// Creates a default cosignature.
		Cosignature() : Version(0)
		{}

		/// Creates a cosignature around \a signerPublicKey and \a signature.
		Cosignature(const Key& signerPublicKey, const catapult::Signature& signature)
				: Version(0)
				, SignerPublicKey(signerPublicKey)
				, Signature(signature)
		{}

	public:
		/// Version.
		uint64_t Version;

		/// Cosignatory public key.
		Key SignerPublicKey;

		/// Cosignatory signature.
		catapult::Signature Signature;
	};

	/// Cosignature detached from an aggregate transaction.
	struct DetachedCosignature : public Cosignature {
	public:
		/// Creates a detached cosignature around \a signerPublicKey, \a signature and \a parentHash.
		DetachedCosignature(const Key& signerPublicKey, const catapult::Signature& signature, const Hash256& parentHash)
				: Cosignature(signerPublicKey, signature)
				, ParentHash(parentHash)
		{}

		/// Creates a detached cosignature around \a cosignature and \a parentHash.
		DetachedCosignature(const Cosignature& cosignature, const Hash256& parentHash)
				: Cosignature(cosignature)
				, ParentHash(parentHash)
		{}

	public:
		/// Hash of the aggregate transaction that is signed by this cosignature.
		Hash256 ParentHash;
	};

#pragma pack(pop)
}}
