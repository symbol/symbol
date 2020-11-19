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
#include "CompactMosaicMap.h"
#include "catapult/utils/Hashers.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace state {

	/// Container holding account balance information.
	class AccountBalances {
	public:
		/// Creates empty account balances.
		AccountBalances();

		/// Copy constructor that makes a deep copy of \a accountBalances.
		AccountBalances(const AccountBalances& accountBalances);

		/// Move constructor that move constructs an account balances from \a accountBalances.
		AccountBalances(AccountBalances&& accountBalances);

	public:
		/// Assignment operator that makes a deep copy of \a accountBalances.
		AccountBalances& operator=(const AccountBalances& accountBalances);

		/// Move assignment operator that assigns \a accountBalances.
		AccountBalances& operator=(AccountBalances&& accountBalances);

	public:
		/// Gets the number of mosaics owned.
		size_t size() const;

		/// Gets a const iterator to the first element of the underlying set.
		CompactMosaicMap::const_iterator begin() const;

		/// Gets a const iterator to the element following the last element of the underlying set.
		CompactMosaicMap::const_iterator end() const;

		/// Gets the optimized mosaic id.
		MosaicId optimizedMosaicId() const;

		/// Gets the balance of the given mosaic (\a mosaicId).
		Amount get(MosaicId mosaicId) const;

	public:
		/// Adds \a amount funds to a given mosaic (\a mosaicId).
		AccountBalances& credit(MosaicId mosaicId, Amount amount);

		/// Subtracts \a amount funds from a given mosaic (\a mosaicId).
		AccountBalances& debit(MosaicId mosaicId, Amount amount);

		/// Optimizes access of the mosaic with \a id.
		void optimize(MosaicId id);

	private:
		CompactMosaicMap m_balances;
		MosaicId m_optimizedMosaicId;
	};
}}
