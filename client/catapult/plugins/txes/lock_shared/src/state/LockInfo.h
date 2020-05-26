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

	/// Lock status.
	enum class LockStatus : uint8_t {
		/// Lock is unused.
		Unused,

		/// Lock was already used.
		Used
	};

	/// Lock info.
	struct LockInfo {
	public:
		static constexpr auto Is_Deactivation_Destructive = true;

	protected:
		/// Creates a default lock info.
		LockInfo()
		{}

		/// Creates a lock info around \a ownerAddress, \a mosaicId, \a amount and \a endHeight.
		LockInfo(const Address& ownerAddress, catapult::MosaicId mosaicId, catapult::Amount amount, Height endHeight)
				: OwnerAddress(ownerAddress)
				, MosaicId(mosaicId)
				, Amount(amount)
				, EndHeight(endHeight)
				, Status(LockStatus::Unused)
		{}

	public:
		/// Owner address.
		Address OwnerAddress;

		/// Mosaic id.
		catapult::MosaicId MosaicId;

		/// Amount.
		catapult::Amount Amount;

		/// Height at which the lock expires.
		Height EndHeight;

		/// Flag indicating whether or not the lock was already used.
		LockStatus Status;

	public:
		/// Returns \c true if lock info is active at \a height.
		constexpr bool isActive(Height height) const {
			return height < EndHeight && LockStatus::Unused == Status;
		}
	};
}}
