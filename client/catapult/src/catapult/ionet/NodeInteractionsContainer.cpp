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

#include "NodeInteractionsContainer.h"
#include "catapult/utils/Functional.h"
#include "catapult/utils/NetworkTime.h"
#include "catapult/utils/TimeSpan.h"
#include <algorithm>

namespace catapult { namespace ionet {

	namespace {
		constexpr bool IsBucketTooOld(Timestamp currentTime, Timestamp creationTime) {
			return currentTime > creationTime
					&& NodeInteractionsContainer::InteractionDuration() <= utils::TimeSpan::FromDifference(currentTime, creationTime);
		}
	}

	utils::TimeSpan NodeInteractionsContainer::BucketDuration() {
		return utils::TimeSpan::FromHours(24);
	}

	utils::TimeSpan NodeInteractionsContainer::InteractionDuration() {
		return utils::TimeSpan::FromHours(7 * 24);
	}

	NodeInteractions NodeInteractionsContainer::interactions(Timestamp timestamp) const {
		NodeInteractions results;
		for (const auto& bucket : m_buckets) {
			if (!IsBucketTooOld(timestamp, bucket.CreationTime)) {
				results.NumSuccesses += bucket.NumSuccesses;
				results.NumFailures += bucket.NumFailures;
			}
		}

		return results;
	}

	void NodeInteractionsContainer::incrementSuccesses(Timestamp timestamp) {
		addInteraction(timestamp, [](auto& bucket) { ++bucket.NumSuccesses; });
	}

	void NodeInteractionsContainer::incrementFailures(Timestamp timestamp) {
		addInteraction(timestamp, [](auto& bucket) { ++bucket.NumFailures; });
	}

	void NodeInteractionsContainer::pruneBuckets(Timestamp timestamp) {
		auto endIter = std::remove_if(m_buckets.begin(), m_buckets.end(), [timestamp](const auto& bucket) {
			return IsBucketTooOld(timestamp, bucket.CreationTime);
		});
		m_buckets.erase(endIter, m_buckets.cend());
	}

	bool NodeInteractionsContainer::shouldCreateNewBucket(Timestamp timestamp) const {
		if (m_buckets.empty())
			return true;

		auto bucketAge = utils::TimeSpan::FromDifference(timestamp, m_buckets.back().CreationTime);
		return BucketDuration() <= bucketAge;
	}

	void NodeInteractionsContainer::addInteraction(Timestamp timestamp, const consumer<NodeInteractionsBucket&>& consumer) {
		if (shouldCreateNewBucket(timestamp))
			m_buckets.push_back(NodeInteractionsBucket(timestamp));

		consumer(m_buckets.back());
	}
}}
