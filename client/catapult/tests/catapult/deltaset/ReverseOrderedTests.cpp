#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using ReverseOrderedMutableTraits = test::ReverseOrderedTraits<MutableTypeTraits<test::MutableTestElement>>;
		using ReverseOrderedMutablePointerTraits =
				test::ReverseOrderedTraits<MutableTypeTraits<std::shared_ptr<test::MutableTestElement>>>;
		using ReverseOrderedImmutableTraits = test::ReverseOrderedTraits<ImmutableTypeTraits<const test::ImmutableTestElement>>;
		using ReverseOrderedImmutablePointerTraits =
				test::ReverseOrderedTraits<ImmutableTypeTraits<std::shared_ptr<const test::ImmutableTestElement>>>;
	}
}}

#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::ReverseOrderedMutableTraits>, DeltaReverseOrderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::ReverseOrderedMutablePointerTraits>, DeltaReverseOrderedMutablePointer); \

#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::ReverseOrderedImmutableTraits>, DeltaReverseOrderedImmutable); \
	MAKE_BASE_SET_TEST( \
			TEST_NAME, \
			test::DeltaTraits<deltaset::ReverseOrderedImmutablePointerTraits>, \
			DeltaReverseOrderedImmutablePointer); \

#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedMutableTraits>, BaseReverseOrderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedMutablePointerTraits>, BaseReverseOrderedMutablePointer); \

#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedImmutableTraits>, BaseReverseOrderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedImmutablePointerTraits>, BaseReverseOrderedImmutablePointer); \

#include "tests/catapult/deltaset/test/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {
/* reverse tests only use ordered base variants */
#define REGISTER_REVERSE_ORDER_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedMutableTraits>, ReverseOrderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedMutablePointerTraits>, ReverseOrderedMutablePointer); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedImmutableTraits>, ReverseOrderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::ReverseOrderedImmutablePointerTraits>, ReverseOrderedImmutablePointer); \

#define DEFINE_REVERSE_ORDER_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_REVERSE_ORDER_TYPES)

	DEFINE_REVERSE_ORDER_TESTS(ReverseOrderedBaseCanIterateThroughSetInReverseOrder) {
		// Arrange:
		auto pReversedBaseSet = TTraits::CreateWithElements(3);
		auto pDelta = pReversedBaseSet->rebase();
		pDelta->emplace("TestElement", 7u);
		pDelta->emplace("TestElement", 4u);
		pReversedBaseSet->commit();
		auto iter = pReversedBaseSet->begin();

		// Assert:
		EXPECT_EQ(test::TestElement("TestElement", 7), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 4), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 2), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 1), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(test::TestElement("TestElement", 0), *TTraits::ToPointerFromStorage(*iter++));
		EXPECT_EQ(pReversedBaseSet->end(), iter);

		// Sanity: the iterator elements are const
		test::AssertConstIterator(*pReversedBaseSet);
	}
}}
