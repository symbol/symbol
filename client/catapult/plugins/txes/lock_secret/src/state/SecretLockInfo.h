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
#include "src/model/LockHashAlgorithm.h"
#include "src/model/LockHashUtils.h"
#include "plugins/txes/lock_shared/src/state/LockInfo.h"
#include "catapult/plugins.h"

namespace catapult { namespace state {

	/// Secret lock info.
	struct PLUGIN_API_DEPENDENCY SecretLockInfo : public LockInfo {
	public:
		/// Creates a default secret lock info.
		SecretLockInfo() : LockInfo()
		{}

		/// Creates a secret lock info around \a ownerAddress, \a mosaicId, \a amount, \a endHeight, \a hashAlgorithm, \a secret
		/// and \a recipientAddress.
		SecretLockInfo(
				const Address& ownerAddress,
				catapult::MosaicId mosaicId,
				catapult::Amount amount,
				Height endHeight,
				model::LockHashAlgorithm hashAlgorithm,
				const Hash256& secret,
				const catapult::Address& recipientAddress)
				: LockInfo(ownerAddress, mosaicId, amount, endHeight)
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
				, RecipientAddress(recipientAddress)
				, CompositeHash()
		{}

	public:
		/// Hash algorithm.
		model::LockHashAlgorithm HashAlgorithm;

		/// Secret.
		Hash256 Secret;

		/// Locked mosaic recipient address.
		Address RecipientAddress;

		/// Composite hash.
		Hash256 CompositeHash;
	};
}}
