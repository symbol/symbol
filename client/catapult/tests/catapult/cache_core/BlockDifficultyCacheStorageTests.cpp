#include "catapult/cache_core/BlockDifficultyCacheStorage.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS BlockDifficultyCacheStorageTests

	namespace {
		using ValueType = state::BlockDifficultyInfo;
		constexpr auto Value_Size = sizeof(Height) + sizeof(Timestamp) + sizeof(Difficulty);
	}

	TEST(TEST_CLASS, CanSaveValue) {
		// Arrange:
		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream("", buffer);

		// - create a random value
		ValueType originalValue(
				test::GenerateRandomValue<Height>(),
				test::GenerateRandomValue<Timestamp>(),
				test::GenerateRandomValue<Difficulty>());

		// Act:
		BlockDifficultyCacheStorage::Save(originalValue, stream);

		// Assert:
		ASSERT_EQ(Value_Size, buffer.size());
		const auto& savedValue = reinterpret_cast<const ValueType&>(*buffer.data());
		EXPECT_EQ(originalValue, savedValue);

		EXPECT_EQ(0u, stream.numFlushes());
	}

	TEST(TEST_CLASS, CanLoadValue) {
		// Arrange:
		BlockDifficultyCache cache(845);
		auto delta = cache.createDelta();

		std::vector<uint8_t> buffer(Value_Size);
		test::FillWithRandomData(buffer);
		mocks::MockMemoryStream stream("", buffer);

		// Act:
		BlockDifficultyCacheStorage::Load(stream, *delta);

		// Assert:
		EXPECT_EQ(1u, delta->size());
		EXPECT_TRUE(delta->contains(reinterpret_cast<const ValueType&>(*buffer.data())));
	}
}}
