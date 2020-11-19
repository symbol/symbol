/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "catapult/model/VerifiableEntityPredicate.h"
#include "catapult/preprocessor.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS VerifiableEntityPredicateTests

	namespace {
		std::unique_ptr<const VerifiableEntity> CreateVerifiableEntity(EntityType type) {
			auto pEntity = std::make_unique<VerifiableEntity>();
			pEntity->Type = type;
			return PORTABLE_MOVE(pEntity);
		}

		constexpr auto Dummy_Transaction = MakeEntityType(BasicEntityType::Transaction, FacilityCode::Multisig, 0xA);
	}

	TEST(TEST_CLASS, NeverFilterPredicateAlwaysReturnsTrue) {
		// Arrange:
		auto predicate = NeverFilter();

		// Act + Assert:
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block_Nemesis)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block_Normal)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Dummy_Transaction)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}

	TEST(TEST_CLASS, HasTypeFilterReturnsTrueForMatchingEntityType) {
		// Arrange:
		auto predicate = HasTypeFilter(Entity_Type_Block_Normal);

		// Act + Assert:
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(Entity_Type_Block_Nemesis)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block_Normal)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(Dummy_Transaction)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}

	TEST(TEST_CLASS, HasBasicTypeFilterReturnsTrueForMatchingBasicEntityType) {
		// Arrange:
		auto predicate = HasBasicTypeFilter(BasicEntityType::Block);

		// Act + Assert:
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block_Nemesis)));
		EXPECT_TRUE(predicate(*CreateVerifiableEntity(Entity_Type_Block_Normal)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(Dummy_Transaction)));
		EXPECT_FALSE(predicate(*CreateVerifiableEntity(static_cast<EntityType>(0x3FFF))));
	}
}}
