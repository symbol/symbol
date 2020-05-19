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
#include "plugins/txes/lock_shared/src/state/LockInfo.h"
#include "catapult/plugins.h"

namespace catapult { namespace state {

	/// Hash lock info.
	struct PLUGIN_API_DEPENDENCY HashLockInfo : public LockInfo {
	public:
		/// Creates a default hash lock info.
		HashLockInfo() : LockInfo()
		{}

		/// Creates a hash lock info around \a ownerAddress, \a mosaicId, \a amount, \a endHeight and \a hash.
		HashLockInfo(
				const Address& ownerAddress,
				catapult::MosaicId mosaicId,
				catapult::Amount amount,
				Height endHeight,
				const Hash256& hash)
				: LockInfo(ownerAddress, mosaicId, amount, endHeight)
				, Hash(hash)
		{}

	public:
		/// Hash.
		Hash256 Hash;
	};
}}
