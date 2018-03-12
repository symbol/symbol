#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using OrderedMutableTraits = test::OrderedTraits<MutableTypeTraits<test::MutableTestElement>>;
		using OrderedMutablePointerTraits = test::OrderedTraits<MutableTypeTraits<std::shared_ptr<test::MutableTestElement>>>;
		using OrderedImmutableTraits = test::OrderedTraits<ImmutableTypeTraits<const test::ImmutableTestElement>>;
		using OrderedImmutablePointerTraits = test::OrderedTraits<ImmutableTypeTraits<std::shared_ptr<const test::ImmutableTestElement>>>;
	}
}}

#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::OrderedMutableTraits>, DeltaOrderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::OrderedMutablePointerTraits>, DeltaOrderedMutablePointer); \

#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::OrderedImmutableTraits>, DeltaOrderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::OrderedImmutablePointerTraits>, DeltaOrderedImmutablePointer); \

#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedMutableTraits>, BaseOrderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedMutablePointerTraits>, BaseOrderedMutablePointer); \

#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedImmutableTraits>, BaseOrderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedImmutablePointerTraits>, BaseOrderedImmutablePointer); \

#include "tests/catapult/deltaset/test/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {
/* forward tests only use ordered base variants */
#define REGISTER_FORWARD_ORDER_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedMutableTraits>, OrderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedMutablePointerTraits>, OrderedMutablePointer); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedImmutableTraits>, OrderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::OrderedImmutablePointerTraits>, OrderedImmutablePointer); \

#define DEFINE_FORWARD_ORDER_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_FORWARD_ORDER_TYPES)

	DEFINE_FORWARD_ORDER_TESTS(OrderedBaseCanIterateThroughSetInOrder) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pBaseSet->rebase();
		pDelta->emplace("TestElement", 7u);
		pDelta->emplace("TestElement", 4u);
		pBaseSet->commit();
		auto iter = pBaseSet->begin();

		// Assert:
		EXPECT_EQ(test::TestElement("TestElement", 0), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 1), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 2), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 4), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 7), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(pBaseSet->end(), iter);

		// Sanity: the iterator elements are const
		test::AssertConstIterator(*pBaseSet);
	}
}}
