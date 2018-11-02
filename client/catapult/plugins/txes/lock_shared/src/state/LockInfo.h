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

namespace catapult { namespace state {

	/// A lock status.
	enum class LockStatus : uint8_t {
		/// Lock is unused.
		Unused,
		/// Lock was already used.
		Used
	};

	/// A lock info.
	struct LockInfo {
	protected:
		/// Creates a default lock info.
		LockInfo()
		{}

		/// Creates a lock info around \a account, \a mosaicId, \a amount and \a height.
		explicit LockInfo(const Key& account, catapult::MosaicId mosaicId, catapult::Amount amount, catapult::Height height)
				: Account(account)
				, MosaicId(mosaicId)
				, Amount(amount)
				, Height(height)
				, Status(LockStatus::Unused)
		{}

	public:
		/// Account.
		Key Account;

		/// Mosaic id.
		catapult::MosaicId MosaicId;

		/// Amount.
		catapult::Amount Amount;

		/// Height at which the lock expires.
		catapult::Height Height;

		/// Flag indicating whether or not the lock was already used.
		LockStatus Status;

	public:
		/// Returns \c true if lock info is active at \a height.
		constexpr bool isActive(catapult::Height height) const {
			return height < Height;
		}
	};
}}
