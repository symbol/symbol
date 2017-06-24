#include "tests/catapult/deltaset/utils/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using OrderedMutableTraits = OrderedTraits<MutableTypeTraits<MutableTestEntity>>;
		using OrderedMutablePointerTraits = OrderedTraits<MutableTypeTraits<std::shared_ptr<MutableTestEntity>>>;
		using OrderedImmutableTraits = OrderedTraits<ImmutableTypeTraits<const ImmutableTestEntity>>;
		using OrderedImmutablePointerTraits = OrderedTraits<ImmutableTypeTraits<std::shared_ptr<const ImmutableTestEntity>>>;
	}

#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<OrderedMutableTraits>, DeltaOrderedMutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<OrderedMutablePointerTraits>, DeltaOrderedMutablePointer); \

#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<OrderedImmutableTraits>, DeltaOrderedImmutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<OrderedImmutablePointerTraits>, DeltaOrderedImmutablePointer); \

#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedMutableTraits>, BaseOrderedMutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedMutablePointerTraits>, BaseOrderedMutablePointer); \

#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedImmutableTraits>, BaseOrderedImmutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedImmutablePointerTraits>, BaseOrderedImmutablePointer);
}}

#include "tests/catapult/deltaset/utils/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {
/* forward tests only use ordered base variants */
#define REGISTER_FORWARD_ORDER_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedMutableTraits>, OrderedMutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedMutablePointerTraits>, OrderedMutablePointer); \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedImmutableTraits>, OrderedImmutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<OrderedImmutablePointerTraits>, OrderedImmutablePointer); \

#define FORWARD_ORDER_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_FORWARD_ORDER_TYPES)

	FORWARD_ORDER_TRAITS_BASED_TEST(OrderedBaseCanIterateThroughSetInOrder) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto pDelta = pBaseSet->rebase();
		pDelta->emplace("TestEntity", 7u);
		pDelta->emplace("TestEntity", 4u);
		pBaseSet->commit();
		auto it = pBaseSet->cbegin();

		// Assert:
		EXPECT_EQ(TestEntity("TestEntity", 0), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 1), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 2), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 4), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(TestEntity("TestEntity", 7), *TTraits::ToPointerFromStorage(*it++));
		EXPECT_EQ(pBaseSet->cend(), it);

		// Sanity: the iterator elements are const
		AssertConstIterator(*pBaseSet);
	}
}}
