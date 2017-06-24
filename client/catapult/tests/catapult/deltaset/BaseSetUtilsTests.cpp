#include "catapult/deltaset/BaseSetUtils.h"
#include "tests/catapult/deltaset/utils/BaseSetTestsInclude.h"
#include "tests/TestHarness.h"
#include <vector>

namespace catapult { namespace deltaset {

#define TEST_CLASS BaseSetUtilsTests

	namespace {
		using NonPointerTraits = BaseTraits<OrderedTraits<MutableTypeTraits<MutableTestEntity>>>;
		using PointerTraits = BaseTraits<OrderedTraits<MutableTypeTraits<std::shared_ptr<MutableTestEntity>>>>;
	}

#define SET_UTILS_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_NonPointer) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonPointerTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Pointer) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PointerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	SET_UTILS_TRAITS_BASED_TEST(ContainsAnyReturnsTrueIfAtLeastOneEntityExists) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDelta = pSet->rebase();
		InsertAll(*pDelta, TTraits::CreateEntities(3));
		auto entities = typename TTraits::EntityVector{
			TTraits::CreateEntity("MyTestEntity", 0), // does not exist in delta
			TTraits::CreateEntity("TestEntity", 1), // exists in delta
			TTraits::CreateEntity("AnotherTestEntity", 123) // does not exist in delta
		};

		// Act:
		auto found = ContainsAny(*pDelta, entities);

		// Assert:
		EXPECT_TRUE(found);
	}

	SET_UTILS_TRAITS_BASED_TEST(ContainsAnyReturnsFalseIfNoEntityExists) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto entities = typename TTraits::EntityVector{
			TTraits::CreateEntity("MyTestEntity", 0), // does not exist in delta
			TTraits::CreateEntity("Blah", 1), // does not exist in delta
			TTraits::CreateEntity("AnotherTestEntity", 123) // does not exist in delta
		};

		// Act:
		auto found = ContainsAny(*pSet, entities);

		// Assert:
		EXPECT_FALSE(found);
	}

	SET_UTILS_TRAITS_BASED_TEST(InsertAllInsertsAllEntities) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDelta = pSet->rebase();
		auto entities = TTraits::CreateEntities(3);

		// Sanity check:
		EXPECT_EQ(0u, pDelta->size());

		// Act:
		InsertAll(*pDelta, entities);

		// Assert:
		EXPECT_EQ(3u, pDelta->size());
		TTraits::AssertContents(*pDelta, entities);
	}

	SET_UTILS_TRAITS_BASED_TEST(RemoveAllRemovesAllEntities) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDelta = pSet->rebase();
		InsertAll(*pDelta, TTraits::CreateEntities(5));
		auto entities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 0),
			TTraits::CreateEntity("TestEntity", 2),
			TTraits::CreateEntity("TestEntity", 4)
		};
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 1),
			TTraits::CreateEntity("TestEntity", 3)
		};

		// Act:
		RemoveAll(*pDelta, entities);

		// Assert:
		EXPECT_EQ(2u, pDelta->size());
		TTraits::AssertContents(*pDelta, expectedEntities);
	}
}}
