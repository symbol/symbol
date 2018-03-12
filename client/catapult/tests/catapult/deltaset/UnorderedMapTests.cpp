#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using UnorderedMapMutableTraits = test::UnorderedMapTraits<MutableTypeTraits<test::MutableTestElement>>;
		using UnorderedMapMutablePointerTraits = test::UnorderedMapTraits<MutableTypeTraits<std::shared_ptr<test::MutableTestElement>>>;
		using UnorderedMapImmutableTraits = test::UnorderedMapTraits<ImmutableTypeTraits<const test::ImmutableTestElement>>;
		using UnorderedMapImmutablePointerTraits =
			test::UnorderedMapTraits<ImmutableTypeTraits<std::shared_ptr<const test::ImmutableTestElement>>>;
	}
}}

#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMapMutableTraits>, DeltaUnorderedMapMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMapMutablePointerTraits>, DeltaUnorderedMapMutablePointer); \

#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMapImmutableTraits>, DeltaUnorderedMapImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::DeltaTraits<deltaset::UnorderedMapImmutablePointerTraits>, DeltaUnorderedMapImmutablePointer); \

#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedMapMutableTraits>, BaseUnorderedMapMutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedMapMutablePointerTraits>, BaseUnorderedMapMutablePointer); \

#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedMapImmutableTraits>, BaseUnorderedMapImmutable); \
	MAKE_BASE_SET_TEST(TEST_NAME, test::BaseTraits<deltaset::UnorderedMapImmutablePointerTraits>, BaseUnorderedMapImmutablePointer); \

#include "tests/catapult/deltaset/test/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {

	DEFINE_DELTA_MUTABLE_TESTS(NonConstFindAllowsElementModification) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto element = TTraits::CreateElement("TestElement", 4);
		pDelta->insert(element);
		pSet->commit();

		// Act: mutate can be called
		auto pDeltaElement = pDelta->find(TTraits::ToKey(element));
		pDeltaElement->mutate();

		// Assert:
		EXPECT_FALSE(std::is_const<decltype(test::Unwrap(pDeltaElement))>());
	}
}}
