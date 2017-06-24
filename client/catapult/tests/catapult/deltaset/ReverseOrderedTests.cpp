#include "tests/catapult/deltaset/utils/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using ReverseOrderedMutableTraits = ReverseOrderedTraits<MutableTypeTraits<MutableTestEntity>>;
		using ReverseOrderedMutablePointerTraits =
				ReverseOrderedTraits<MutableTypeTraits<std::shared_ptr<MutableTestEntity>>>;
		using ReverseOrderedImmutableTraits = ReverseOrderedTraits<ImmutableTypeTraits<const ImmutableTestEntity>>;
		using ReverseOrderedImmutablePointerTraits =
				ReverseOrderedTraits<ImmutableTypeTraits<std::shared_ptr<const ImmutableTestEntity>>>;
	}

#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<ReverseOrderedMutableTraits>, DeltaReverseOrderedMutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<ReverseOrderedMutablePointerTraits>, DeltaReverseOrderedMutablePointer); \

#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<ReverseOrderedImmutableTraits>, DeltaReverseOrderedImmutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<ReverseOrderedImmutablePointerTraits>, DeltaReverseOrderedImmutablePointer); \

#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedMutableTraits>, BaseReverseOrderedMutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedMutablePointerTraits>, BaseReverseOrderedMutablePointer); \

#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedImmutableTraits>, BaseReverseOrderedImmutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedImmutablePointerTraits>, BaseReverseOrderedImmutablePointer);
}}

#include "tests/catapult/deltaset/utils/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {
/* reverse tests only use ordered base variants */
#define REGISTER_REVERSE_ORDER_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedMutableTraits>, ReverseOrderedMutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedMutablePointerTraits>, ReverseOrderedMutablePointer); \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedImmutableTraits>, ReverseOrderedImmutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<ReverseOrderedImmutablePointerTraits>, ReverseOrderedImmutablePointer); \

#define REVERSE_ORDER_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_REVERSE_ORDER_TYPES)

	REVERSE_ORDER_TRAITS_BASED_TEST(ReverseOrderedBaseCanIterateThroughSetInReverseOrder) {
		// Arrange:
		auto pReversedBaseSet = TTraits::CreateWithEntities(3);
		auto pDelta = pReversedBaseSet->rebase();
		pDelta->emplace("TestEntity", 7u);
		pDelta->emplace("TestEntity", 4u);
		pReversedBaseSet->commit();
		auto it = pReversedBaseSet->cbegin();

		// Assert:
		EXPECT_EQ(TestEntity("TestEntity", 7), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 4), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 2), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 1), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 0), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(pReversedBaseSet->cend(), it);

		// Sanity: the iterator elements are const
		AssertConstIterator(*pReversedBaseSet);
	}
}}
