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

#include "catapult/model/WeakEntityInfo.h"
#include "catapult/model/Block.h"
#include "catapult/utils/HexParser.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS WeakEntityInfoTests

	namespace {
		// region asserts

		template<typename TEntity>
		void AssertAreEqual(const WeakEntityInfoT<TEntity>& info, const VerifiableEntity& entity, const Hash256& hash, const char* tag) {
			// Assert:
			ASSERT_TRUE(info.isSet()) << tag;
			EXPECT_EQ(&entity, &info.entity()) << tag;

			ASSERT_TRUE(info.isHashSet()) << tag;
			EXPECT_EQ(&hash, &info.hash()) << tag;

			EXPECT_FALSE(info.isAssociatedBlockHeaderSet()) << tag;
		}

		template<typename TEntity>
		void AssertAreEqual(
				const WeakEntityInfoT<TEntity>& info,
				const VerifiableEntity& entity,
				const Hash256& hash,
				const BlockHeader& blockHeader,
				const char* tag) {
			// Assert:
			ASSERT_TRUE(info.isSet()) << tag;
			EXPECT_EQ(&entity, &info.entity()) << tag;

			ASSERT_TRUE(info.isHashSet()) << tag;
			EXPECT_EQ(&hash, &info.hash()) << tag;

			ASSERT_TRUE(info.isAssociatedBlockHeaderSet()) << tag;
			EXPECT_EQ(&blockHeader, &info.associatedBlockHeader()) << tag;
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateUnsetWeakEntityInfo) {
		// Act:
		WeakEntityInfo info;

		// Assert:
		EXPECT_FALSE(info.isSet());
		EXPECT_FALSE(info.isHashSet());
		EXPECT_FALSE(info.isAssociatedBlockHeaderSet());
	}

	TEST(TEST_CLASS, CanCreateWeakEntityInfoAroundEntity) {
		// Arrange:
		VerifiableEntity entity;

		// Act:
		WeakEntityInfo info(entity);

		// Assert:
		ASSERT_TRUE(info.isSet());
		EXPECT_EQ(&entity, &info.entity());

		EXPECT_FALSE(info.isHashSet());
		EXPECT_FALSE(info.isAssociatedBlockHeaderSet());
	}

	TEST(TEST_CLASS, CanCreateWeakEntityInfoAroundEntityAndHash) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;

		// Act:
		WeakEntityInfo info(entity, hash);

		// Assert:
		AssertAreEqual(info, entity, hash, "info");
	}

	TEST(TEST_CLASS, CanCreateWeakEntityInfoAroundEntityAndHashAndAssociatedBlockHeader) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;
		BlockHeader blockHeader;

		// Act:
		WeakEntityInfo info(entity, hash, blockHeader);

		// Assert:
		AssertAreEqual(info, entity, hash, blockHeader, "info");
	}

	// endregion

	// region assign

	TEST(TEST_CLASS, CanAssignWeakEntityInfo) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;
		BlockHeader blockHeader;

		WeakEntityInfo info1;
		WeakEntityInfo info2(entity, hash, blockHeader);

		// Sanity:
		EXPECT_FALSE(info1.isSet());

		// Act:
		info1 = info2;

		// Assert:
		AssertAreEqual(info1, entity, hash, blockHeader, "info1");
		AssertAreEqual(info2, entity, hash, blockHeader, "info2");
	}

	// endregion

	// region type

	TEST(TEST_CLASS, CanAccessEntityType) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;
		entity.Type = static_cast<EntityType>(0x5432);

		// Act:
		WeakEntityInfo info(entity, hash);

		// Assert:
		EXPECT_EQ(static_cast<EntityType>(0x5432), info.type());
	}

	// endregion

	// region cast

	TEST(TEST_CLASS, CanConvertToStronglyTypedInfoWithoutAssociatedBlockHeader) {
		// Arrange:
		Block block;
		Hash256 hash;
		WeakEntityInfo info(block, hash);

		// Act:
		auto blockInfo = info.cast<Block>();

		// Assert:
		AssertAreEqual(info, block, hash, "info");
		AssertAreEqual(blockInfo, block, hash, "blockInfo");

		auto isEntityTyped = std::is_same_v<const Block&, decltype(blockInfo.entity())>;
		EXPECT_TRUE(isEntityTyped);
	}

	TEST(TEST_CLASS, CanConvertToStronglyTypedInfoWithAssociatedBlockHeader) {
		// Arrange:
		Block block;
		Hash256 hash;
		BlockHeader blockHeader;
		WeakEntityInfo info(block, hash, blockHeader);

		// Act:
		auto blockInfo = info.cast<Block>();

		// Assert:
		AssertAreEqual(info, block, hash, blockHeader, "info");
		AssertAreEqual(blockInfo, block, hash, blockHeader, "blockInfo");

		auto isEntityTyped = std::is_same_v<const Block&, decltype(blockInfo.entity())>;
		EXPECT_TRUE(isEntityTyped);
	}

	// endregion

	// region equality operators

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy", "diff-block-header" };
		}

		std::unordered_map<std::string, WeakEntityInfo> GenerateEqualityInstanceMap() {
			VerifiableEntity entity1;
			VerifiableEntity entity2;
			Hash256 hash1;
			Hash256 hash2;
			BlockHeader blockHeader;

			return {
				{ "default", WeakEntityInfo(entity1, hash1) },
				{ "copy", WeakEntityInfo(entity1, hash1) },
				{ "diff-block-header", WeakEntityInfo(entity1, hash1, blockHeader) },

				{ "diff-entity", WeakEntityInfo(entity2, hash1) },
				{ "diff-hash", WeakEntityInfo(entity1, hash2) },
				{ "diff-both", WeakEntityInfo(entity2, hash2) },
				{ "unset", WeakEntityInfo() }
			};
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, UnsetInfosAreEqual) {
		// Arrange:
		WeakEntityInfo unsetInfo1;
		WeakEntityInfo unsetInfo2;

		// Assert:
		EXPECT_TRUE(unsetInfo1.operator==(unsetInfo2));
		EXPECT_FALSE(unsetInfo1.operator!=(unsetInfo2));
	}

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputUnsetEntityInfo) {
		// Arrange:
		WeakEntityInfo info;

		// Act:
		auto str = test::ToString(info);

		// Assert:
		EXPECT_EQ("WeakEntityInfo (unset)", str);
	}

	TEST(TEST_CLASS, CanOutputSetEntityInfo) {
		// Arrange:
		VerifiableEntity entity;
		entity.Size = 121;
		entity.Version = 2;
		entity.Network = NetworkIdentifier::Zero;
		entity.Type = Entity_Type_Block_Nemesis;

		Hash256 hash = utils::ParseByteArray<Hash256>("C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470");
		WeakEntityInfo info(entity, hash);

		// Act:
		auto str = test::ToString(info);

		// Assert:
		EXPECT_EQ("Block_Nemesis (v2) with size 121 [C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470]", str);
	}

	// endregion
}}
