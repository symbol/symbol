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

#include "FinalizationPatchingSubscriber.h"
#include "finalization/src/io/PrevoteChainStorage.h"
#include "catapult/io/BlockStorageCache.h"

namespace catapult { namespace chain {

	FinalizationPatchingSubscriber::FinalizationPatchingSubscriber(
			io::PrevoteChainStorage& prevoteChainStorage,
			const io::BlockStorageCache& blockStorage,
			const consumer<model::BlockRange&&>& blockRangeConsumer)
			: m_prevoteChainStorage(prevoteChainStorage)
			, m_blockStorage(blockStorage)
			, m_blockRangeConsumer(blockRangeConsumer)
	{}

	namespace {
		bool Contains(const io::BlockStorageCache& blockStorage, Height height, const Hash256& hash) {
			auto storageView = blockStorage.view();
			return storageView.chainHeight() >= height && hash == storageView.loadBlockElement(height)->EntityHash;
		}
	}

	void FinalizationPatchingSubscriber::notifyFinalizedBlock(const model::FinalizationRound& round, Height height, const Hash256& hash) {
		if (!Contains(m_blockStorage, height, hash) && m_prevoteChainStorage.contains(round, { height, hash })) {
			// load all blocks up to and including finalized height
			auto blockRange = m_prevoteChainStorage.load(round, height);
			m_blockRangeConsumer(std::move(blockRange));
		}

		m_prevoteChainStorage.remove(round);
	}
}}
