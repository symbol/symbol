/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "src/state/MosaicDefinition.h"
#include "src/model/MosaicProperties.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicDefinitionTests

	namespace {
		constexpr Height Default_Height(345);

		void AssertDefaultRequiredProperties(const model::MosaicProperties& properties) {
			EXPECT_FALSE(properties.is(model::MosaicFlags::Supply_Mutable));
			EXPECT_FALSE(properties.is(model::MosaicFlags::Transferable));
			EXPECT_FALSE(properties.is(model::MosaicFlags::Levy_Mutable));
			EXPECT_EQ(0u, properties.divisibility());
		}

		void AssertCustomOptionalProperties(const model::MosaicProperties& expectedProperties, const model::MosaicProperties& properties) {
			EXPECT_EQ(expectedProperties.duration(), properties.duration());
		}

		MosaicDefinition CreateMosaicDefinition(uint64_t duration) {
			auto owner = test::GenerateRandomData<Key_Size>();
			return MosaicDefinition(Default_Height, owner, test::CreateMosaicPropertiesWithDuration(BlockDuration(duration)));
		}
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateMosaicDefinition_DefaultProperties) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		auto properties = model::MosaicProperties::FromValues({});

		// Act:
		MosaicDefinition definition(Height(877), owner, properties);

		// Assert:
		EXPECT_EQ(Height(877), definition.height());
		EXPECT_EQ(owner, definition.owner());
		AssertDefaultRequiredProperties(definition.properties());
		AssertCustomOptionalProperties(properties, definition.properties());
	}

	TEST(TEST_CLASS, CanCreateMosaicDefinition_CustomProperties) {
		// Arrange:
		auto owner = test::GenerateRandomData<Key_Size>();
		auto properties = test::CreateMosaicPropertiesWithDuration(BlockDuration(3));

		// Act:
		MosaicDefinition definition(Height(877), owner, properties);

		// Assert:
		EXPECT_EQ(Height(877), definition.height());
		EXPECT_EQ(owner, definition.owner());
		AssertDefaultRequiredProperties(definition.properties());
		AssertCustomOptionalProperties(properties, definition.properties());
	}

	// endregion

	// region isEternal

	TEST(TEST_CLASS, IsEternalReturnsTrueIfMosaicDefinitionHasEternalDuration) {
		// Arrange:
		auto definition = CreateMosaicDefinition(Eternal_Artifact_Duration.unwrap());

		// Assert:
		EXPECT_TRUE(definition.isEternal());
	}

	TEST(TEST_CLASS, IsEternalReturnsFalseIfMosaicDefinitionDoesNotHaveEternalDuration) {
		// Arrange:
		for (auto duration : { 1u, 2u, 1000u, 10000u, 1'000'000'000u }) {
			auto definition = CreateMosaicDefinition(duration);

			// Assert:
			EXPECT_FALSE(definition.isEternal()) << "duration " << duration;
		}
	}

	// endregion

	// region isActive

	namespace {
		void AssertActiveOrNot(BlockDuration::ValueType duration, const std::vector<Height::ValueType>& heights, bool expectedResult) {
			// Arrange: creation height is 345
			auto definition = CreateMosaicDefinition(duration);

			// Assert:
			for (auto height : heights)
				EXPECT_EQ(expectedResult, definition.isActive(Height(height))) << "at height " << height;
		}
	}

	TEST(TEST_CLASS, IsActiveReturnsTrueIfMosaicDefinitionIsActive) {
		// Assert:
		auto duration = 57u;
		auto height = Default_Height.unwrap();
		AssertActiveOrNot(duration, { height, height + 1, height + 22, height + duration - 2, height + duration - 1 }, true);
	}

	TEST(TEST_CLASS, IsActiveReturnsTrueIfMosaicDefinitionIsEternal) {
		// Assert:
		auto duration = Eternal_Artifact_Duration.unwrap();
		auto height = Default_Height.unwrap();
		AssertActiveOrNot(duration, { height - 1, height, height + 1, 500u, 5000u, std::numeric_limits<Height::ValueType>::max() }, true);
	}

	TEST(TEST_CLASS, IsActiveReturnsFalseIfMosaicDefinitionIsNotActive) {
		// Assert:
		auto duration = 57u;
		auto height = Default_Height.unwrap();
		AssertActiveOrNot(duration, { 1u, height - 2, height - 1, height + duration, height + duration + 1, height + 10'000 }, false);
	}

	// endregion

	// region isExpired

	namespace {
		void AssertExpiredOrNot(BlockDuration::ValueType duration, const std::vector<Height::ValueType>& heights, bool expectedResult) {
			// Arrange: creation height is 345
			auto definition = CreateMosaicDefinition(duration);

			// Assert:
			for (auto height : heights)
				EXPECT_EQ(expectedResult, definition.isExpired(Height(height))) << "at height " << height;
		}
	}

	TEST(TEST_CLASS, IsExpiredReturnsTrueIfMosaicDefinitionIsExpired) {
		// Assert:
		auto duration = 57u;
		auto height = Default_Height.unwrap();
		AssertExpiredOrNot(duration, { height + duration, height + duration + 1, height + 10'000 }, true);
	}

	TEST(TEST_CLASS, IsExpiredReturnsFalseIfMosaicDefinitionIsEternal) {
		// Assert:
		auto duration = Eternal_Artifact_Duration.unwrap();
		auto height = Default_Height.unwrap();
		AssertExpiredOrNot(duration, { 1, height - 1, height, height + 1, 5000u, std::numeric_limits<Height::ValueType>::max() }, false);
	}

	TEST(TEST_CLASS, IsExpiredReturnsFalseIfMosaicDefinitionIsNotExpired) {
		// Assert:
		auto duration = 57u;
		auto height = Default_Height.unwrap();
		AssertExpiredOrNot(duration, { 1, height - 1, height, height + 1, height + duration - 2, height + duration - 1 }, false);
	}

	// endregion
}}
