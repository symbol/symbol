#include "tests/test/core/mocks/MemoryBasedStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

	namespace {
		struct MemoryBasedGuard {
			std::string name() const { return std::string(); }
		};

		struct MemoryBasedTraits {
			using Guard = MemoryBasedGuard;
			using StorageType = mocks::MemoryBasedStorage;

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
}}

#define STORAGE_TESTS_CLASS_NAME MemoryBasedStorageTests
#define STORAGE_TESTS_TRAITS_NAME MemoryBasedTraits

#include "BlockStorageTests.h"

#undef STORAGE_TESTS_TRAITS_NAME
#undef STORAGE_TESTS_CLASS_NAME
