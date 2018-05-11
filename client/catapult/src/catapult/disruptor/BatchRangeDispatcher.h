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
#include "ConsumerDispatcher.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/SpinLock.h"
#include <unordered_map>
#include <vector>

namespace catapult { namespace disruptor {

	/// Batches entity ranges for processing by a ConsumerDispatcher.
	template<typename TAnnotatedEntityRange>
	class BatchRangeDispatcher {
	private:
		using EntityRange = decltype(TAnnotatedEntityRange::Range);

		struct RangeGroupKey {
		public:
			Key SourcePublicKey;
			InputSource Source;

		public:
			friend bool operator==(const RangeGroupKey& lhs, const RangeGroupKey& rhs) {
				return lhs.SourcePublicKey == rhs.SourcePublicKey && lhs.Source == rhs.Source;
			}
		};

		struct RangeGroupKeyHasher {
			size_t operator()(const RangeGroupKey& key) const {
				return utils::ArrayHasher<Key>()(key.SourcePublicKey);
			}
		};

		using GroupedRangesMap = std::unordered_map<RangeGroupKey, std::vector<EntityRange>, RangeGroupKeyHasher>;

	public:
		/// Creates a batch range dispatcher around \a dispatcher.
		explicit BatchRangeDispatcher(ConsumerDispatcher& dispatcher) : m_dispatcher(dispatcher)
		{}

	public:
		/// Queues processing of \a range from \a source.
		void queue(TAnnotatedEntityRange&& range, InputSource source) {
			utils::SpinLockGuard guard(m_lock);
			m_rangesMap[{ range.SourcePublicKey, source }].push_back(std::move(range.Range));
		}

		/// Dispatches all queued elements to the underlying dispatcher.
		void dispatch() {
			GroupedRangesMap rangesMap;

			{
				utils::SpinLockGuard guard(m_lock);
				rangesMap = std::move(m_rangesMap);
			}

			for (auto& pair : rangesMap) {
				auto mergedRange = EntityRange::MergeRanges(std::move(pair.second));
				m_dispatcher.processElement(ConsumerInput({ std::move(mergedRange), pair.first.SourcePublicKey }, pair.first.Source));
			}
		}

	public:
		/// Returns \c true if no ranges are currently queued.
		bool empty() const {
			utils::SpinLockGuard guard(m_lock);
			return m_rangesMap.empty();
		}

	private:
		ConsumerDispatcher& m_dispatcher;
		GroupedRangesMap m_rangesMap;
		mutable utils::SpinLock m_lock;
	};
}}
