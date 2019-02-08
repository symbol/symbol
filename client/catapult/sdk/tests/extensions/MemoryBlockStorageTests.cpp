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

#include "src/extensions/MemoryBlockStorage.h"
#include "tests/test/core/BlockStorageTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS MemoryBlockStorageTests

	namespace {
		struct MemoryTraits {
			struct Guard {
				std::string name() const {
					return std::string();
				}
			};
			using StorageType = MemoryBlockStorage;

			static std::unique_ptr<StorageType> OpenStorage(const std::string&) {
				// load and copy the nemesis into storage
				auto nemesisBlockElement = test::BlockToBlockElement(test::GetNemesisBlock());
				nemesisBlockElement.GenerationHash = test::GetNemesisGenerationHash();
				return std::make_unique<StorageType>(nemesisBlockElement);
			}

			static std::unique_ptr<StorageType> PrepareStorage(const std::string& destination, Height height = Height()) {
				auto pStorage = OpenStorage(destination);

				// abuse drop blocks to fake current height...
				// note: since we will want to save next block at height, we need to drop all
				// after `height-1` due to check in saveBlock()
				if (Height() != height)
					pStorage->dropBlocksAfter(Height(height.unwrap() - 1));

				return pStorage;
			}
		};
	}

	DEFINE_BLOCK_STORAGE_TESTS(MemoryTraits)
}}
