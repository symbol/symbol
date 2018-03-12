#include "mongo/src/ExternalCacheStorageBuilder.h"
#include "tests/test/local/mocks/MockExternalCacheStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS ExternalCacheStorageBuilderTests

	namespace {
		template<size_t CacheId>
		void AddSubStorageWithId(ExternalCacheStorageBuilder& builder) {
			builder.add(std::make_unique<mocks::MockExternalCacheStorage<CacheId>>());
		}
	}

	TEST(TEST_CLASS, CanCreateEmptyStorage) {
		// Arrange:
		ExternalCacheStorageBuilder builder;

		// Act:
		auto pStorage = builder.build();

		// Assert:
		EXPECT_EQ("{}", pStorage->name());
	}

	TEST(TEST_CLASS, CanCreateStorageWithSingleSubStorage) {
		// Arrange:
		ExternalCacheStorageBuilder builder;
		AddSubStorageWithId<5>(builder);

		// Act:
		auto pStorage = builder.build();

		// Assert:
		EXPECT_EQ("{ SimpleCache }", pStorage->name());
	}

	TEST(TEST_CLASS, CanCreateStorageWithMultipleSubStorage) {
		// Arrange:
		ExternalCacheStorageBuilder builder;
		AddSubStorageWithId<5>(builder);
		AddSubStorageWithId<2>(builder);
		AddSubStorageWithId<6>(builder);

		// Act:
		auto pStorage = builder.build();

		// Assert:
		EXPECT_EQ("{ SimpleCache, SimpleCache, SimpleCache }", pStorage->name());
	}

	TEST(TEST_CLASS, CannotAddMultipleStoragesWithSameId) {
		// Arrange:
		ExternalCacheStorageBuilder builder;
		AddSubStorageWithId<5>(builder);
		AddSubStorageWithId<2>(builder);
		AddSubStorageWithId<6>(builder);

		// Act + Assert:
		EXPECT_THROW(AddSubStorageWithId<5>(builder), catapult_invalid_argument);
	}
}}
