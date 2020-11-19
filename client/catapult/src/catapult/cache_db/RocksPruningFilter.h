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
#include <memory>

namespace rocksdb { class CompactionFilter; }

namespace catapult { namespace cache {

	/// Possible modes of rocks pruning filter.
	enum class FilterPruningMode {
		/// Disabled, skip allocation of underlying filter implementation.
		Disabled,

		/// Enabled, allocate underlying filter implementation.
		Enabled
	};

	/// Rocks pruning filter.
	class RocksPruningFilter final {
	public:
		/// Creates rocks pruning filter with optional \a pruningMode.
		explicit RocksPruningFilter(FilterPruningMode pruningMode = FilterPruningMode::Disabled);

		/// Destroys rocks pruning filter.
		~RocksPruningFilter();

	public:
		/// Gets the underlying compaction filter.
		rocksdb::CompactionFilter* compactionFilter();

		/// Gets the pruning boundary.
		uint64_t pruningBoundary() const;

		/// Gets the number of pruned entries since last prune.
		size_t numRemoved() const;

	public:
		/// Sets the pruning boundary.
		void setPruningBoundary(uint64_t pruningBoundary);

	private:
		class RocksPruningFilterImpl;
		std::unique_ptr<RocksPruningFilterImpl> m_pImpl;
	};
}}
