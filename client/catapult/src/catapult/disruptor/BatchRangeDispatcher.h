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
