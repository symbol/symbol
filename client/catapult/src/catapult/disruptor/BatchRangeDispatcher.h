#pragma once
#include "ConsumerDispatcher.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/SpinLock.h"
#include <vector>

namespace catapult { namespace disruptor {

	/// Batches entity ranges for processing by a ConsumerDispatcher.
	template<typename TEntityRange>
	class BatchRangeDispatcher {
	private:
		using SpinLockGuard = std::lock_guard<utils::SpinLock>;
		using SourceToRangesMap = std::vector<std::vector<TEntityRange>>;

	public:
		/// Creates a batch range dispatcher around \a dispatcher.
		explicit BatchRangeDispatcher(ConsumerDispatcher& dispatcher)
				: m_dispatcher(dispatcher)
		{}

	public:
		/// Queues processing of \a range from \a source.
		void queue(TEntityRange&& range, InputSource source) {
			SpinLockGuard guard(m_lock);
			auto sourceIndex = utils::to_underlying_type(source);
			if (m_sourceToRangesMap.size() <= sourceIndex)
				m_sourceToRangesMap.resize(sourceIndex + 1);

			m_sourceToRangesMap[sourceIndex].push_back(std::move(range));
		}

		/// Dispatches all queued elements to the underlying dispatcher.
		void dispatch() {
			SourceToRangesMap sourceToRangesMap;

			{
				SpinLockGuard guard(m_lock);
				sourceToRangesMap = std::move(m_sourceToRangesMap);
			}

			for (auto i = 0u; i < sourceToRangesMap.size(); ++i) {
				auto& ranges = sourceToRangesMap[i];
				if (ranges.empty())
					continue;

				auto mergedRange = TEntityRange::MergeRanges(std::move(ranges));
				m_dispatcher.processElement(ConsumerInput(std::move(mergedRange), static_cast<InputSource>(i)));
			}
		}

	public:
		/// Returns \c true if no ranges are currently queued.
		bool empty() const {
			SpinLockGuard guard(m_lock);
			return std::all_of(m_sourceToRangesMap.cbegin(), m_sourceToRangesMap.cend(), [](const auto& ranges) {
				return ranges.empty();
			});
		}

	private:
		ConsumerDispatcher& m_dispatcher;
		SourceToRangesMap m_sourceToRangesMap;
		mutable utils::SpinLock m_lock;
	};
}}
