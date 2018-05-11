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

#include "tests/test/core/mocks/MockMemoryBasedStorage.h"
#include "tests/catapult/io/test/BlockStorageTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS MemoryBasedStorageTests

	namespace {
		struct MemoryBasedGuard {
			std::string name() const {
				return std::string();
			}
		};

		struct MemoryBasedTraits {
			using Guard = MemoryBasedGuard;
			using StorageType = mocks::MockMemoryBasedStorage;

			static std::unique_ptr<StorageType> OpenStorage(const std::string&) {
				return std::make_unique<StorageType>();
			}

			static std::unique_ptr<StorageType> PrepareStorage(const std::string& destination, Height height = Height()) {
				auto pStorage = OpenStorage(destination);

				if (Height() != height)
					// abuse drop blocks to fake current height...
					// note: since we will want to save next block at height, we need to drop all
					// after `height-1` due to check in saveBlock()
					pStorage->dropBlocksAfter(Height(height.unwrap() - 1));

				return pStorage;
			}
		};
	}

	DEFINE_BLOCK_STORAGE_TESTS(MemoryBasedTraits)
}}
