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
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace catapult { namespace test {

	/// Aggregates processing information across multiple threads.
	template<typename TTraits>
	class BasicMultiThreadedState {
	private:
		using LockGuard = std::lock_guard<std::mutex>;

	public:
		/// Gets the total number of items processed.
		size_t counter() const {
			LockGuard guard(m_mutex);
			size_t total = 0;
			for (const auto& pair : m_counts)
				total += pair.second;

			return total;
		}

		/// Gets the total number of items processed by each thread.
		std::vector<size_t> threadCounters() const {
			LockGuard guard(m_mutex);
			std::vector<size_t> counters;
			for (const auto& pair : m_counts)
				counters.push_back(pair.second);

			return counters;
		}

		/// Gets the number of unique items processed.
		size_t numUniqueItems() const {
			LockGuard guard(m_mutex);
			std::unordered_set<uint64_t> values;
			for (const auto& pair : m_valueThreadPairs) {
				if (values.cend() != values.find(pair.first))
					continue;

				values.insert(pair.first);
			}

			return values.size();
		}

		/// Gets the sorted and reduced thread ids used during processing.
		std::vector<std::thread::id> sortedAndReducedThreadIds() const {
			LockGuard guard(m_mutex);

			// 1. sort all (value, thread_id) pairs by value (used as a unique item id)
			auto pairs = m_valueThreadPairs;
			std::sort(pairs.begin(), pairs.end(), [](const auto& l, const auto& r) { return l.first < r.first; });

			// 2. only add thread_ids that are different from the thread id for the previous item
			//    (items are expected to be batched together, so there shouldn't be any interleaving)
			std::vector<std::thread::id> threadIds;
			for (const auto& pair : pairs) {
				if (!threadIds.empty() && threadIds.back() == pair.second)
					continue;

				threadIds.push_back(pair.second);
			}

			return threadIds;
		}

	public:
		/// Processes \a item.
		void process(const typename TTraits::ItemType& item) {
			LockGuard guard(m_mutex);
			auto id = std::this_thread::get_id();
			auto iter = m_counts.find(id);
			if (m_counts.end() == iter) {
				m_counts.emplace(id, 0);
				iter = m_counts.find(id);
			}

			iter->second++;
			m_valueThreadPairs.emplace_back(TTraits::GetValue(item), id);
		}

	private:
		mutable std::mutex m_mutex;
		std::unordered_map<std::thread::id, size_t> m_counts;
		std::vector<std::pair<uint64_t, std::thread::id>> m_valueThreadPairs;
	};
}}
