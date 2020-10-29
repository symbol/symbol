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

#include "catapult/model/SizeChecker.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS SizeCheckerTests

	namespace {
#pragma pack(push, 1)

		struct VariableSizedEntity {
		public:
			explicit VariableSizedEntity(uint8_t extraSize) : ExtraSize(extraSize)
			{}

		public:
			uint32_t Size;
			uint8_t ExtraSize;

		public:
			static uint64_t CalculateRealSize(const VariableSizedEntity& entity) {
				return sizeof(VariableSizedEntity) + entity.ExtraSize;
			}
		};

#pragma pack(pop)
	}

	TEST(TEST_CLASS, IsSizeValidReturnsTrueWhenEntitySizeIsCorrect_WithNoVariableData) {
		// Arrange:
		auto entity = VariableSizedEntity(0);
		entity.Size = sizeof(VariableSizedEntity);

		// Act + Assert:
		EXPECT_TRUE(IsSizeValidT(entity));
	}

	TEST(TEST_CLASS, IsSizeValidReturnsTrueWhenEntitySizeIsCorrect_WithVariableData) {
		// Arrange:
		auto entity = VariableSizedEntity(123);
		entity.Size = sizeof(VariableSizedEntity) + 123;

		// Act + Assert:
		EXPECT_TRUE(IsSizeValidT(entity));
	}

	TEST(TEST_CLASS, IsSizeValidReturnsFalseWhenEntitySizeIsLessThanEntityHeaderSize) {
		// Arrange: construct an incomplete VariableSizedEntity that will cause an AV when accessing ExtraSize
		std::vector<uint8_t> buffer(sizeof(uint32_t));
		auto& entity = reinterpret_cast<VariableSizedEntity&>(buffer[0]);
		entity.Size = sizeof(uint32_t);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValidT(entity));
	}

	TEST(TEST_CLASS, IsSizeValidReturnsFalseWhenEntitySizeIsTooSmall) {
		// Arrange:
		auto entity = VariableSizedEntity(123);
		entity.Size = sizeof(VariableSizedEntity) + 123 - 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValidT(entity));
	}

	TEST(TEST_CLASS, IsSizeValidReturnsFalseWhenEntitySizeIsTooLarge) {
		// Arrange:
		auto entity = VariableSizedEntity(123);
		entity.Size = sizeof(VariableSizedEntity) + 123 + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValidT(entity));
	}
}}
