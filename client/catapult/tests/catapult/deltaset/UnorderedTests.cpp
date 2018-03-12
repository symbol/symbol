#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using UnorderedMutableTraits = test::UnorderedTraits<MutableTypeTraits<test::MutableTestElement>>;
		using UnorderedMutablePointerTraits = test::UnorderedTraits<MutableTypeTraits<std::shared_ptr<test::MutableTestElement>>>;
		using UnorderedImmutableTraits = test::UnorderedTraits<ImmutableTypeTraits<const test::ImmutableTestElement>>;
		using UnorderedImmutablePointerTraits =
			test::UnorderedTraits<ImmutableTypeTraits<std::shared_ptr<const test::ImmutableTestElement>>>;
	}

#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMutableTraits>, DeltaUnorderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMutablePointerTraits>, DeltaUnorderedMutablePointer); \

#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedImmutableTraits>, DeltaUnorderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedImmutablePointerTraits>, DeltaUnorderedImmutablePointer); \

#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedMutableTraits>, BaseUnorderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedMutablePointerTraits>, BaseUnorderedMutablePointer); \

#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedImmutableTraits>, BaseUnorderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedImmutablePointerTraits>, BaseUnorderedImmutablePointer);
}}

#include "tests/catapult/deltaset/test/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {
/* hasher tests only use unordered delta variants */
#define REGISTER_HASHER_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMutableTraits>, DeltaUnorderedMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMutablePointerTraits>, DeltaUnorderedMutablePointer); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedImmutableTraits>, DeltaUnorderedImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedImmutablePointerTraits>, DeltaUnorderedImmutablePointer); \

#define DEFINE_HASHER_TESTS(TEST_NAME) DEFINE_BASE_SET_TESTS(TEST_NAME, REGISTER_HASHER_TYPES)

	DEFINE_HASHER_TESTS(SuppliedHasherIsUsed) {
		// Arrange:
		auto pDelta = TTraits::Create();
		auto element = TTraits::CreateElement("", 0);

		// Act:
		pDelta->insert(element);

		// Assert:
		EXPECT_LE(1u, TTraits::ToPointer(element)->HasherCallCount);
	}
}}
