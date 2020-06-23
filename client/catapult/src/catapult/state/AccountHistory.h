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
#include "HeightIndexedHistoryMap.h"
#include "catapult/model/PinnedVotingKey.h"
#include <vector>

namespace catapult { namespace state {

	/// Represents an account history.
	/// \note This is required for finalization voting.
	class AccountHistory {
	public:
		/// Gets the balance history.
		const HeightIndexedHistoryMap<Amount>& balance() const;

		/// Gets the vrf public key history.
		const HeightIndexedHistoryMap<Key>& vrfPublicKey() const;

		/// Gets the voting public keys history.
		const HeightIndexedHistoryMap<std::vector<model::PinnedVotingKey>>& votingPublicKeys() const;

		/// Returns \c true if any historical balance is at least \a minAmount.
		bool anyAtLeast(Amount minAmount) const;

	public:
		/// Adds \a balance at \a height.
		void add(Height height, Amount balance);

		/// Adds \a vrfPublicKey at \a height.
		void add(Height height, const Key& vrfPublicKey);

		/// Adds \a votingPublicKeys at \a height.
		void add(Height height, const std::vector<model::PinnedVotingKey>& votingPublicKeys);

		/// Prunes all balances less than \a height.
		/// \note Prune will never change the result of balance queries at or after \a height.
		void prune(Height height);

	private:
		HeightIndexedHistoryMap<Amount> m_heightBalanceMap;
		HeightIndexedHistoryMap<Key> m_heightVrfPublicKeyMap;
		HeightIndexedHistoryMap<std::vector<model::PinnedVotingKey>> m_heightVotingPublicKeysMap;
	};
}}
