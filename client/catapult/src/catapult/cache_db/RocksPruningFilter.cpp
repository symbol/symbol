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

#include "RocksPruningFilter.h"
#include "RocksInclude.h"
#include <cstring>

namespace catapult { namespace cache {

	class RocksPruningFilter::RocksPruningFilterImpl final : public rocksdb::CompactionFilter {
	public:
		RocksPruningFilterImpl()
				: m_compactionBoundary(0)
				, m_numRemoved(0)
		{}

	public:
		const char* Name() const override {
			return "pruning compaction filter";
		}

		// must be thread-safe if compaction_filter is used
		bool Filter(int, const rocksdb::Slice& key, const rocksdb::Slice&, std::string*, bool*) const override {
			// 1. assume that key is prepended by filter uint64_t (do not assume specific key type to allow maximum reusability)
			// 2. skip keys too small to contain filter value (e.g. special 'size' key)
			if (key.size() < Special_Key_Max_Length)
				return false;

			// due to short std::string optimization, std::string data is not guaranteed to be aligned
			uint64_t value;
			std::memcpy(&value, key.data(), sizeof(uint64_t));

			if (value < m_compactionBoundary) {
				++m_numRemoved;
				return true;
			}

			return false;
		}

		bool IgnoreSnapshots() const override {
			return true;
		}

	public:
		uint64_t pruningBoundary() const {
			return m_compactionBoundary.load();
		}

		size_t numRemoved() const {
			return m_numRemoved.load();
		}

	public:
		void setPruningBoundary(uint64_t compactionBoundary) {
			m_compactionBoundary = compactionBoundary;
			m_numRemoved = 0;
		}

	private:
		std::atomic<uint64_t> m_compactionBoundary;
		mutable std::atomic<size_t> m_numRemoved;
	};

	RocksPruningFilter::RocksPruningFilter(FilterPruningMode mode) {
		if (FilterPruningMode::Enabled == mode)
			m_pImpl = std::make_unique<RocksPruningFilterImpl>();
	}

	RocksPruningFilter::~RocksPruningFilter() = default;

	rocksdb::CompactionFilter* RocksPruningFilter::compactionFilter() {
		return m_pImpl.get();
	}

	uint64_t RocksPruningFilter::pruningBoundary() const {
		return m_pImpl ? m_pImpl->pruningBoundary() : 0;
	}

	size_t RocksPruningFilter::numRemoved() const {
		return m_pImpl ? m_pImpl->numRemoved() : 0;
	}

	void RocksPruningFilter::setPruningBoundary(uint64_t compactionBoundary) {
		if (m_pImpl)
			m_pImpl->setPruningBoundary(compactionBoundary);
	}
}}
