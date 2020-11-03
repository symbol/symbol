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
#include "catapult/model/HeightHashPair.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/subscribers/FinalizationSubscriber.h"

namespace catapult { namespace io { class BlockStorageCache; } }

namespace catapult { namespace chain {

	/// Manager of backed up prevote chains.
	class PrevoteChainBackups {
	public:
		virtual ~PrevoteChainBackups() = default;

	public:
		/// Returns \c true if backed up chain for \a round contains \a heightHashPair.
		virtual bool contains(const model::FinalizationRound& round, const model::HeightHashPair& heightHashPair) const = 0;

		/// Loads backed up chain for \a round up to \a maxHeight.
		virtual model::BlockRange load(const model::FinalizationRound& round, Height maxHeight) const = 0;

		/// Removes backed up chain for \a round.
		virtual void remove(const model::FinalizationRound& round) = 0;
	};

	/// Finalization subscriber that patches the local chain with a prevoted chain when the prevoted chain but not the local chain
	/// contains the finalized block.
	class FinalizationPatchingSubscriber : public subscribers::FinalizationSubscriber {
	public:
		/// Creates a subscriber around \a prevoteChainBackups, \a blockStorage and \a blockRangeConsumer
		FinalizationPatchingSubscriber(
				PrevoteChainBackups& prevoteChainBackups,
				const io::BlockStorageCache& blockStorage,
				const consumer<model::BlockRange&&>& blockRangeConsumer);

	public:
		void notifyFinalizedBlock(const model::FinalizationRound& round, Height height, const Hash256& hash) override;

	private:
		PrevoteChainBackups& m_prevoteChainBackups;
		const io::BlockStorageCache& m_blockStorage;
		consumer<model::BlockRange&&> m_blockRangeConsumer;
	};
}}
