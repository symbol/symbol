#include "tests/catapult/deltaset/utils/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using UnorderedMutableTraits = UnorderedTraits<MutableTypeTraits<MutableTestEntity>>;
		using UnorderedMutablePointerTraits = UnorderedTraits<MutableTypeTraits<std::shared_ptr<MutableTestEntity>>>;
		using UnorderedImmutableTraits = UnorderedTraits<ImmutableTypeTraits<const ImmutableTestEntity>>;
		using UnorderedImmutablePointerTraits = UnorderedTraits<ImmutableTypeTraits<std::shared_ptr<const ImmutableTestEntity>>>;
	}

#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMutableTraits>, DeltaUnorderedMutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMutablePointerTraits>, DeltaUnorderedMutablePointer); \

#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedImmutableTraits>, DeltaUnorderedImmutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedImmutablePointerTraits>, DeltaUnorderedImmutablePointer); \

#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedMutableTraits>, BaseUnorderedMutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedMutablePointerTraits>, BaseUnorderedMutablePointer); \

#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedImmutableTraits>, BaseUnorderedImmutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedImmutablePointerTraits>, BaseUnorderedImmutablePointer);
}}

#include "tests/catapult/deltaset/utils/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {
/* hasher tests only use unordered delta variants */
#define REGISTER_HASHER_TYPES(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMutableTraits>, DeltaUnorderedMutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMutablePointerTraits>, DeltaUnorderedMutablePointer); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedImmutableTraits>, DeltaUnorderedImmutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedImmutablePointerTraits>, DeltaUnorderedImmutablePointer); \

#define HASHER_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_HASHER_TYPES)

	HASHER_TRAITS_BASED_TEST(SuppliedHasherIsUsed) {
		// Arrange:
		auto pDelta = TTraits::Create();
		auto entity = TTraits::CreateEntity("", 0);

		// Act:
		pDelta->insert(entity);

		// Assert:
		EXPECT_LE(1u, TTraits::ToPointer(entity)->HasherCallCount);
	}
}}
