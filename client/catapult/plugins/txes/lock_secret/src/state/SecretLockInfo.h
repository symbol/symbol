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

namespace catapult { namespace state {

	/// A secret lock info.
	struct SecretLockInfo : public LockInfo {
	public:
		/// Creates a default secret lock info.
		SecretLockInfo() : LockInfo()
		{}

		/// Creates a secret lock info around \a account, \a mosaicId, \a amount, \a height, \a hashAlgorithm, \a secret and \a recipient.
		SecretLockInfo(
				const Key& account,
				catapult::MosaicId mosaicId,
				catapult::Amount amount,
				catapult::Height height,
				model::LockHashAlgorithm hashAlgorithm,
				const Hash256& secret,
				const catapult::Address& recipient)
				: LockInfo(account, mosaicId, amount, height)
				, HashAlgorithm(hashAlgorithm)
				, Secret(secret)
				, Recipient(recipient)
				, CompositeHash()
		{}

	public:
		/// Hash algorithm.
		model::LockHashAlgorithm HashAlgorithm;

		/// Secret.
		Hash256 Secret;

		/// Recipient of the locked mosaic.
		catapult::Address Recipient;

		/// Composite hash.
		Hash256 CompositeHash;
	};
}}
