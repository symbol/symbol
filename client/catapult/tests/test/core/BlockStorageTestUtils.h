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
#include "catapult/io/BlockStorage.h"

namespace catapult { namespace test {

	/// Seeds \a storage with blocks starting at \a startHeight ending at \a endHeight inclusive.
	void SeedBlocks(io::BlockStorage& storage, Height startHeight, Height endHeight);

	/// Seeds \a storage with \a numBlocks blocks (storage will contain blocks with heights 1 - numBlocks).
	void SeedBlocks(io::BlockStorage& storage, size_t numBlocks);

	/// Creates block element from \a block with a random transaction hash.
	model::BlockElement CreateBlockElementForSaveTests(const model::Block& block);

	/// Loads the block element at \a height along with its statements from \a storage.
	std::shared_ptr<const model::BlockElement> LoadBlockElementWithStatements(const io::BlockStorage& storage, Height height);

	// region PrepareStorageWithBlocks

	/// Context for holding storage and optional storage guard.
	template<typename TTraits>
	struct StorageContextT {
		std::unique_ptr<typename TTraits::Guard> pTempDirectoryGuard;
		std::unique_ptr<typename TTraits::StorageType> pStorage;

	public:
		typename TTraits::StorageType& operator*() {
			return *pStorage;
		}

		typename TTraits::StorageType* operator->() {
			return pStorage.get();
		}
	};

	/// Prepares storage context by seeding storage with \a numBlocks
	template<typename TTraits>
	auto PrepareStorageWithBlocks(size_t numBlocks) {
		StorageContextT<TTraits> context;
		context.pTempDirectoryGuard = std::make_unique<typename TTraits::Guard>();
		context.pStorage = TTraits::PrepareStorage(context.pTempDirectoryGuard->name());
		SeedBlocks(*context.pStorage, numBlocks);
		return context;
	}

	// endregion
}}
