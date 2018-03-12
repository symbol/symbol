#include "catapult/model/VerifiableEntityPredicate.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS VerifiableEntityPredicateTests

	namespace {
		std::unique_ptr<const VerifiableEntity> CreateVerifiableEntity(EntityType type) {
			auto pEntity = std::make_unique<VerifiableEntity>();
			pEntity->Type = type;
			return std::move(pEntity);
		}

		constexpr auto Dummy_Transaction = MakeEntityType(BasicEntityType::Transaction, FacilityCode::Multisig, 0xA);
	}

	TEST(TEST_CLASS, NeverFilterPredicateAlwaysReturnsTrue) {
		// Arrange:
		auto predicate = NeverFilter();

		// Act + Assert:
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Nemesis_Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Dummy_Transaction)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}

	TEST(TEST_CLASS, HasTypeFilterReturnsTrueForMatchingEntityType) {
		// Arrange:
		auto predicate = HasTypeFilter(Entity_Type_Block);

		// Act + Assert:
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(Entity_Type_Nemesis_Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(Dummy_Transaction)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}

	TEST(TEST_CLASS, HasBasicTypeFilterReturnsTrueForMatchingBasicEntityType) {
		// Arrange:
		auto predicate = HasBasicTypeFilter(BasicEntityType::Block);

		// Act + Assert:
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Nemesis_Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(Dummy_Transaction)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}
}}
