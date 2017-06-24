#include "tests/catapult/deltaset/utils/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		using UnorderedMapMutableTraits = UnorderedMapTraits<MutableTypeTraits<MutableTestEntity>>;
		using UnorderedMapMutablePointerTraits = UnorderedMapTraits<MutableTypeTraits<std::shared_ptr<MutableTestEntity>>>;
		using UnorderedMapImmutableTraits = UnorderedMapTraits<ImmutableTypeTraits<const ImmutableTestEntity>>;
		using UnorderedMapImmutablePointerTraits = UnorderedMapTraits<ImmutableTypeTraits<std::shared_ptr<const ImmutableTestEntity>>>;
	}

	#define REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME) \
		REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMapMutableTraits>, DeltaUnorderedMapMutable); \
		REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMapMutablePointerTraits>, DeltaUnorderedMapMutablePointer); \

	#define REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
		REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMapImmutableTraits>, DeltaUnorderedMapImmutable); \
		REGISTER_TEST(TEST_NAME, DeltaTraits<UnorderedMapImmutablePointerTraits>, DeltaUnorderedMapImmutablePointer); \

	#define REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME) \
		REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedMapMutableTraits>, BaseUnorderedMapMutable); \
		REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedMapMutablePointerTraits>, BaseUnorderedMapMutablePointer); \

	#define REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME) \
		REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedMapImmutableTraits>, BaseUnorderedMapImmutable); \
		REGISTER_TEST(TEST_NAME, BaseTraits<UnorderedMapImmutablePointerTraits>, BaseUnorderedMapImmutablePointer);
}}

#include "tests/catapult/deltaset/utils/BaseSetTestsImpl.h"

namespace catapult { namespace deltaset {

	DELTA_MUTABLE_TRAITS_BASED_TEST(NonConstFindAllowsElementModification) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto entity = TTraits::CreateEntity("TestEntity", 4);
		pDelta->insert(entity);
		pSet->commit();

		// Act: mutate can be called
		auto pDeltaEntity = pDelta->find(TTraits::ToKey(entity));
		pDeltaEntity->mutate();

		// Assert:
		EXPECT_FALSE(std::is_const<decltype(Unwrap(pDeltaEntity))>());
	}
}}
