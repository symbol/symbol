#include "catapult/model/WeakEntityInfo.h"
#include "catapult/model/Block.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	namespace {
		template<typename TEntity>
		void AssertAreEqual(
				const WeakEntityInfoT<TEntity>& info,
				const VerifiableEntity& entity,
				const Hash256& hash,
				const char* tag) {
			// Assert:
			ASSERT_TRUE(info.isSet()) << tag;
			EXPECT_EQ(&entity, &info.entity()) << tag;

			ASSERT_TRUE(info.isHashSet()) << tag;
			EXPECT_EQ(&hash, &info.hash()) << tag;
		}
	}

	TEST(WeakEntityInfoTests, CanCreateUnsetWeakEntityInfo) {
		// Act:
		WeakEntityInfo info;

		// Assert:
		EXPECT_FALSE(info.isSet());
		EXPECT_FALSE(info.isHashSet());
	}

	TEST(WeakEntityInfoTests, CanCreateWeakEntityInfoWithoutHash) {
		// Arrange:
		VerifiableEntity entity;

		// Act:
		WeakEntityInfo info(entity);

		// Assert:
		EXPECT_TRUE(info.isSet());
		EXPECT_FALSE(info.isHashSet());
	}

	TEST(WeakEntityInfoTests, CanCreateWeakEntityInfo) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;

		// Act:
		WeakEntityInfo info(entity, hash);

		// Assert:
		AssertAreEqual(info, entity, hash, "info");
	}

	TEST(WeakEntityInfoTests, CanAssignWeakEntityInfo) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;

		WeakEntityInfo info1;
		WeakEntityInfo info2(entity, hash);

		// Sanity:
		EXPECT_FALSE(info1.isSet());

		// Act:
		info1 = info2;

		// Assert:
		AssertAreEqual(info1, entity, hash, "info1");
		AssertAreEqual(info2, entity, hash, "info2");
	}

	TEST(WeakEntityInfoTests, CanAccessEntityType) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;
		entity.Type = EntityType::Transfer;

		// Act:
		WeakEntityInfo info(entity, hash);

		// Assert:
		EXPECT_EQ(EntityType::Transfer, info.type());
	}

	namespace {
		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "copy" };
		}

		std::unordered_map<std::string, WeakEntityInfo> GenerateEqualityInstanceMap() {
			VerifiableEntity entity1;
			VerifiableEntity entity2;
			Hash256 hash1;
			Hash256 hash2;

			return {
				{ "default", WeakEntityInfo(entity1, hash1) },
				{ "copy", WeakEntityInfo(entity1, hash1) },

				{ "diff-entity", WeakEntityInfo(entity2, hash1) },
				{ "diff-hash", WeakEntityInfo(entity1, hash2) },
				{ "diff-both", WeakEntityInfo(entity2, hash2) },
				{ "unset", WeakEntityInfo() }
			};
		}
	}

	TEST(WeakEntityInfoTests, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(WeakEntityInfoTests, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(WeakEntityInfoTests, UnsetInfosAreEqual) {
		// Arrange:
		WeakEntityInfo unsetInfo1;
		WeakEntityInfo unsetInfo2;

		// Assert:
		EXPECT_TRUE(unsetInfo1 == unsetInfo2);
		EXPECT_FALSE(unsetInfo1 != unsetInfo2);
	}

	TEST(WeakEntityInfoTests, CanConvertToStronglyTypedInfo) {
		// Arrange:
		Block block;
		Hash256 hash;
		WeakEntityInfo info(block, hash);

		// Act:
		auto blockInfo = info.cast<Block>();

		// Assert:
		AssertAreEqual(info, block, hash, "info");
		AssertAreEqual(blockInfo, block, hash, "blockInfo");

		auto isEntityTyped = std::is_same<const Block&, decltype(blockInfo.entity())>::value;
		EXPECT_TRUE(isEntityTyped);
	}

	TEST(WeakEntityInfoTests, CanOutputUnsetEntityInfo) {
		// Arrange:
		WeakEntityInfo info;

		// Act:
		auto str = test::ToString(info);

		// Assert:
		EXPECT_EQ("WeakEntityInfo (unset)", str);
	}

	TEST(WeakEntityInfoTests, CanOutputSetEntityInfo) {
		// Arrange:
		VerifiableEntity entity;
		entity.Size = 121;
		entity.Type = EntityType::Transfer;
		entity.Version = MakeVersion(NetworkIdentifier::Zero, 2);

		Hash256 hash = test::ToArray<Hash256_Size>("C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470");

		WeakEntityInfo info(entity, hash);

		// Act:
		auto str = test::ToString(info);

		// Assert:
		EXPECT_EQ("Transfer (v2) with size 121 [C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470]", str);
	}
}}
