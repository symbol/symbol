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
#include "catapult/utils/Hashers.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace cache {

	/// Map of accounts to weights.
	class AccountWeights {
	public:
		/// Creates account weights.
		AccountWeights();

	public:
		/// Gets the number of unique accounts.
		size_t size() const;

		/// Gets the sum of all weights.
		uint64_t totalWeight() const;

		/// Gets the weight for the account with public \a key.
		uint64_t weight(const Key& key) const;

	public:
		/// Increases the weight for the account with public \a key by \a delta.
		void increment(const Key& key, uint64_t delta);

		/// Decreases the weight for the account with public \a key by \a delta.
		void decrement(const Key& key, uint64_t delta);

		/// Resets the weights.
		void reset();

	private:
		std::unordered_map<Key, uint64_t, utils::ArrayHasher<Key>> m_accountWeights;
		uint64_t m_totalWeight;
	};
}}
