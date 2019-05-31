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
#include "catapult/utils/Hashers.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace cache {

	/// Counters for tracking how often an account is used.
	class AccountCounters {
	public:
		/// Creates account counters.
		AccountCounters();

	public:
		/// Gets the number of unique accounts used.
		size_t size() const;

		/// Gets the sum of all use counts.
		size_t deepSize() const;

		/// Gets the use count for the account with public \a key.
		size_t count(const Key& key) const;

	public:
		/// Increases the use count for the account with public \a key.
		void increment(const Key& key);

		/// Decreases the use count for the account with public \a key.
		void decrement(const Key& key);

		/// Resets the counters.
		void reset();

	private:
		std::unordered_map<Key, size_t, utils::ArrayHasher<Key>> m_accountCounters;
		size_t m_totalUseCount;
	};
}}
