#include "catapult/model/WeakEntityInfo.h"
#include "catapult/model/Block.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS WeakEntityInfoTests

	namespace {
		template<typename TEntity>
		void AssertAreEqual(const WeakEntityInfoT<TEntity>& info, const VerifiableEntity& entity, const Hash256& hash, const char* tag) {
			// Assert:
			ASSERT_TRUE(info.isSet()) << tag;
			EXPECT_EQ(&entity, &info.entity()) << tag;

			ASSERT_TRUE(info.isHashSet()) << tag;
			EXPECT_EQ(&hash, &info.hash()) << tag;
		}
	}

	TEST(TEST_CLASS, CanCreateUnsetWeakEntityInfo) {
		// Act:
		WeakEntityInfo info;

		// Assert:
		EXPECT_FALSE(info.isSet());
		EXPECT_FALSE(info.isHashSet());
	}

	TEST(TEST_CLASS, CanCreateWeakEntityInfoWithoutHash) {
		// Arrange:
		VerifiableEntity entity;

		// Act:
		WeakEntityInfo info(entity);

		// Assert:
		EXPECT_TRUE(info.isSet());
		EXPECT_FALSE(info.isHashSet());
	}

	TEST(TEST_CLASS, CanCreateWeakEntityInfo) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;

		// Act:
		WeakEntityInfo info(entity, hash);

		// Assert:
		AssertAreEqual(info, entity, hash, "info");
	}

	TEST(TEST_CLASS, CanAssignWeakEntityInfo) {
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

	TEST(TEST_CLASS, CanAccessEntityType) {
		// Arrange:
		VerifiableEntity entity;
		Hash256 hash;
		entity.Type = static_cast<model::EntityType>(0x5432);

		// Act:
		WeakEntityInfo info(entity, hash);

		// Assert:
		EXPECT_EQ(static_cast<model::EntityType>(0x5432), info.type());
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

	TEST(TEST_CLASS, OperatorEqualReturnsTrueOnlyForEqualValues) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueOnlyForUnequalValues) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, UnsetInfosAreEqual) {
		// Arrange:
		WeakEntityInfo unsetInfo1;
		WeakEntityInfo unsetInfo2;

		// Assert:
		EXPECT_TRUE(unsetInfo1 == unsetInfo2);
		EXPECT_FALSE(unsetInfo1 != unsetInfo2);
	}

	TEST(TEST_CLASS, CanConvertToStronglyTypedInfo) {
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
		entity.Type = Entity_Type_Nemesis_Block;
		entity.Version = MakeVersion(NetworkIdentifier::Zero, 2);

		Hash256 hash = test::ToArray<Hash256_Size>("C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470");
		WeakEntityInfo info(entity, hash);

		// Act:
		auto str = test::ToString(info);

		// Assert:
		EXPECT_EQ("Nemesis_Block (v2) with size 121 [C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470]", str);
	}
}}
