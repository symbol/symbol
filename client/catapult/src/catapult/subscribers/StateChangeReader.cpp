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

#include "StateChangeReader.h"
#include "StateChangeInfo.h"
#include "StateChangeSubscriber.h"
#include "SubscriberOperationTypes.h"
#include "catapult/cache/CacheChangesStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/exceptions.h"

namespace catapult { namespace subscribers {

	namespace {
		model::ChainScore ReadChainScore(io::InputStream& inputStream) {
			auto scoreHigh = io::Read64(inputStream);
			auto scoreLow = io::Read64(inputStream);
			return model::ChainScore(scoreHigh, scoreLow);
		}

		cache::CacheChanges ReadCacheChanges(io::InputStream& inputStream, const CacheChangesStorages& cacheChangesStorages) {
			cache::CacheChanges::MemoryCacheChangesContainer loadedChanges;
			for (const auto& pStorage : cacheChangesStorages) {
				auto cacheId = pStorage->id();
				if (loadedChanges.size() <= cacheId)
					loadedChanges.resize(cacheId + 1);

				loadedChanges[cacheId] = pStorage->loadAll(inputStream);
			}

			return cache::CacheChanges(std::move(loadedChanges));
		}

		void ReadAndNotifyScoreChange(io::InputStream& inputStream, StateChangeSubscriber& subscriber) {
			auto chainScore = ReadChainScore(inputStream);
			subscriber.notifyScoreChange(chainScore);
		}

		void ReadAndNotifyStateChange(
				io::InputStream& inputStream,
				const CacheChangesStorages& cacheChangesStorages,
				StateChangeSubscriber& subscriber) {
			auto chainScoreDelta = model::ChainScore::Delta(static_cast<int64_t>(io::Read64(inputStream)));
			auto height = io::Read<Height>(inputStream);
			auto cacheChanges = ReadCacheChanges(inputStream, cacheChangesStorages);
			subscriber.notifyStateChange({ std::move(cacheChanges), chainScoreDelta, height });
		}
	}

	void ReadNextStateChange(
			io::InputStream& inputStream,
			const CacheChangesStorages& cacheChangesStorages,
			StateChangeSubscriber& subscriber) {
		auto operationType = static_cast<StateChangeOperationType>(io::Read8(inputStream));

		switch (operationType) {
		case StateChangeOperationType::Score_Change:
			return ReadAndNotifyScoreChange(inputStream, subscriber);
		case StateChangeOperationType::State_Change:
			return ReadAndNotifyStateChange(inputStream, cacheChangesStorages, subscriber);
		}

		CATAPULT_THROW_INVALID_ARGUMENT_1("invalid state change operation type", static_cast<uint16_t>(operationType));
	}
}}
