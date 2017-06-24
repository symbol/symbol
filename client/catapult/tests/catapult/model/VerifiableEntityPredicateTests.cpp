#include "catapult/model/VerifiableEntityPredicate.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	namespace {
		std::unique_ptr<const VerifiableEntity> CreateVerifiableEntity(EntityType type) {
			auto pEntity = std::make_unique<VerifiableEntity>();
			pEntity->Type = type;
			return std::move(pEntity);
		}
	}

	TEST(VerifiableEntityPredicateTests, NeverFilterPredicateAlwaysReturnsTrue) {
		// Arrange:
		auto predicate = NeverFilter();

		// Act + Assert:
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(EntityType::Nemesis_Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(EntityType::Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(EntityType::Transfer)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}

	TEST(VerifiableEntityPredicateTests, HasTypeFilterReturnsTrueForMatchingEntityType) {
		// Arrange:
		auto predicate = HasTypeFilter(EntityType::Block);

		// Act + Assert:
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(EntityType::Nemesis_Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(EntityType::Block)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(EntityType::Transfer)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}

	TEST(VerifiableEntityPredicateTests, HasBasicTypeFilterReturnsTrueForMatchingBasicEntityType) {
		// Arrange:
		auto predicate = HasBasicTypeFilter(BasicEntityType::Block);

		// Act + Assert:
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(EntityType::Nemesis_Block)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(EntityType::Block)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(EntityType::Transfer)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}
}}
