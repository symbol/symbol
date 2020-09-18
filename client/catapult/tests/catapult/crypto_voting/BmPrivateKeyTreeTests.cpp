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

#include "catapult/crypto_voting/BmPrivateKeyTree.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/catapult/crypto_voting/test/BmTreeTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS BmPrivateKeyTreeTests

	namespace {
		constexpr auto Start_Key = BmKeyIdentifier{ 8, 4 };
		constexpr auto End_Key = BmKeyIdentifier{ 13, 5 };
		constexpr BmOptions Default_Options{ 7, Start_Key, End_Key };

		constexpr auto L1_Payload_Start = test::BmTreeSizes::CalculateLevelOnePayloadStart(0);
		constexpr auto L2_Payload_Start = test::BmTreeSizes::CalculateLevelTwoPayloadStart(6);
		constexpr auto Full_L1_Size = test::BmTreeSizes::CalculateFullLevelOneSize(6);
		constexpr auto Full_L2_Size = test::BmTreeSizes::CalculateFullLevelTwoSize(6, 7);

		auto GenerateKeyPair() {
			return VotingKeyPair::FromPrivate(VotingPrivateKey::Generate(test::RandomByte));
		}

		// region test context

		class TestContext {
		public:
			TestContext() : m_tree(BmPrivateKeyTree::Create(GenerateKeyPair(), m_storage, Default_Options))
			{}

			TestContext(TestContext& originalContext)
					: m_tree(BmPrivateKeyTree::FromStream(CopyInto(originalContext.m_storage, m_storage)))
			{}

		public:
			auto& storage() {
				return m_storage;
			}

			auto& tree() {
				return m_tree;
			}

		private:
			static mocks::MockSeekableMemoryStream& CopyInto(
					const mocks::MockSeekableMemoryStream& source,
					mocks::MockSeekableMemoryStream& dest) {
				source.copyTo(dest);
				return dest;
			}

		private:
			mocks::MockSeekableMemoryStream m_storage;
			BmPrivateKeyTree m_tree;
		};

		// endregion
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateEmptyTree) {
		// Arrange:
		auto rootKeyPair = GenerateKeyPair();
		auto expectedPublicKey = rootKeyPair.publicKey();
		mocks::MockSeekableMemoryStream storage;

		// Act:
		auto tree = BmPrivateKeyTree::Create(std::move(rootKeyPair), storage, Default_Options);

		// Assert:
		EXPECT_EQ(expectedPublicKey, tree.rootPublicKey());
		test::AssertOptions(Default_Options, tree.options());

		// - (storage) all level one keys are created upon initial creation
		ASSERT_EQ(Full_L1_Size, storage.buffer().size());
		test::AssertZeroedKeys(storage.buffer(), L1_Payload_Start, 6, {}, "L1");
	}

	TEST(TEST_CLASS, CanLoadTreeFromStream) {
		// Arrange:
		auto rootKeyPair = GenerateKeyPair();
		auto expectedPublicKey = rootKeyPair.publicKey();
		mocks::MockSeekableMemoryStream storage;
		{
			auto originalTree = BmPrivateKeyTree::Create(std::move(rootKeyPair), storage, Default_Options);
			storage.seek(0);
		}

		// Act:
		auto tree = BmPrivateKeyTree::FromStream(storage);

		// Assert:
		EXPECT_EQ(expectedPublicKey, tree.rootPublicKey());
		test::AssertOptions(Default_Options, tree.options());
	}

	// endregion

	// region canSign / sign - success

	namespace {
		void AssertCanSign(const BmKeyIdentifier& keyIdentifier) {
			// Arrange:
			TestContext context;

			// Act + Assert:
			test::AssertCanSign(context.tree(), keyIdentifier);
		}
	}

	TEST(TEST_CLASS, CanSignWithStartKey) {
		AssertCanSign(Start_Key);
	}

	TEST(TEST_CLASS, CanSignWithMiddleKey) {
		AssertCanSign({ (Start_Key.BatchId + End_Key.BatchId) / 2, (Start_Key.KeyId + End_Key.KeyId) / 2 });
	}

	TEST(TEST_CLASS, CanSignWithEndKey) {
		AssertCanSign(End_Key);
	}

	TEST(TEST_CLASS, CanResignDifferentMessageWithSameKey) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		for (auto i = 0u; i < 5u; ++i)
			test::AssertCanSign(context.tree(), { 10, 3 });
	}

	TEST(TEST_CLASS, CanResignSameMessageWithSameKey) {
		// Arrange:
		TestContext context;
		auto messageBuffer = test::GenerateRandomArray<10>();
		auto referenceSignature = context.tree().sign({ 10, 3 }, messageBuffer);

		for (auto i = 0u; i < 5u; ++i) {
			// Act:
			auto signature = context.tree().sign({ 10, 3 }, messageBuffer);

			// Assert:
			EXPECT_EQ(referenceSignature, signature) << i;
		}
	}

	TEST(TEST_CLASS, CanSignWithMultipleIncreasingValidKeys) {
		// Arrange:
		TestContext context;
		std::initializer_list<BmKeyIdentifier> keyIdentifiers{
			{ 8, 4 },
			{ 8, 5 }, // KeyId incremenent
			{ 9, 0 }, // BatchId increment
			{ 11, 0 }, // BatchId skip
			{ 13, 3 } // BatchId and KeyId skip
		};

		// Act:
		for (const auto& keyIdentifier : keyIdentifiers)
			test::AssertCanSign(context.tree(), keyIdentifier);
	}

	// endregion

	// region canSign / sign - failure

	namespace {
		void AssertCannotSign(const BmKeyIdentifier& keyIdentifier) {
			// Arrange:
			TestContext context;

			// Act + Assert:
			test::AssertCannotSign(context.tree(), keyIdentifier);
		}
	}

	TEST(TEST_CLASS, CannotSignWithKeyOutsideOfRange) {
		AssertCanSign({ 10, Default_Options.Dilution - 1 });
		AssertCannotSign({ 10, Default_Options.Dilution });
	}

	TEST(TEST_CLASS, CannotSignWithKeyBelowStart) {
		AssertCannotSign({ Start_Key.BatchId - 1, Start_Key.KeyId });
		AssertCannotSign({ Start_Key.BatchId, Start_Key.KeyId - 1 });
	}

	TEST(TEST_CLASS, CannotSignWithKeyAboveStart) {
		AssertCannotSign({ End_Key.BatchId + 1, End_Key.KeyId });
		AssertCannotSign({ End_Key.BatchId, End_Key.KeyId + 1});
	}

	TEST(TEST_CLASS, CannotSignWithInvalidKey) {
		AssertCannotSign({ 10, BmKeyIdentifier::Invalid_Id });
		AssertCannotSign({ BmKeyIdentifier::Invalid_Id, 3 });
	}

	TEST(TEST_CLASS, CannotSignWithEarlierKey) {
		// Arrange:
		TestContext context;
		test::AssertCanSign(context.tree(), { 10, 3 });

		// Act + Assert:
		test::AssertCannotSign(context.tree(), { 10, 2 });
		test::AssertCannotSign(context.tree(), { 9, 3 });

		// Sanity:
		test::AssertCanSign(context.tree(), { 10, 3 });
	}

	// endregion

	// region wipe - success

	TEST(TEST_CLASS, SignCreatesSubLevels) {
		// Arrange:
		TestContext context;

		// Act:
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());

		// Assert:
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, {}, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, {}, "L2");
	}

	TEST(TEST_CLASS, CanWipeSingleKey) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 9, 2 });

		// Assert: (8,) (9,) L1 and (9,0) (9,1) (9,2) L2 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, CanWipeConsecutiveKeys) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 9, 3 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 9, 2 });
		context.tree().wipe({ 9, 3 });

		// Assert: (8,) (9,) L1 and (9,0) (9,1) (9,2) (9,3) L2 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 3, 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, CanWipeNonConsecutiveKeyFromSameBatch) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 9, 5 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 9, 2 });
		context.tree().wipe({ 9, 5 });

		// Assert: (8,) (9,) L1 and (9,0) (9,1) (9,2) (9,3) (9,4) (9,5) L2 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 1, 2, 3, 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, CanWipeNonConsecutiveKeyFromDifferentBatches) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 13, 2 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 9, 2 });
		context.tree().wipe({ 13, 2 });

		// Assert: (8,) (9,) (10,) (11,) (12,) (13,) L1 and (13,0) (13,1) (13,2) L2 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 0, 1, 2, 3, 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, CanWipeEndKey) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 13, 2 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe(End_Key);

		// Assert: (8,) (9,) (10,) (11,) (12,) (13,) L1 and (13,0) (13,1) (13,2) (13,3) (13,4) (13,5) L2 are cleared
		//         (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 0, 1, 2, 3, 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 1, 2, 3, 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, CanWipeSingleKeyMultipleTimes) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());

		// Act:
		for (auto i = 0u; i < 5; ++ i)
			context.tree().wipe({ 9, 2 });

		// Assert: (8,) (9,) L1 and (9,0) (9,1) (9,2) L2 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, CanWipeSingleLevel) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 13, 2 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 13, BmKeyIdentifier::Invalid_Id });

		// Assert: (8,) (9,) (10,) (11,) (12,) (13,) L1 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 0, 1, 2, 3, 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, {}, "L2");
	}

	// endregion

	// region wipe - failure

	namespace {
		void AssertCanWipe(BmPrivateKeyTree& tree, const BmKeyIdentifier& keyIdentifier) {
			// Act + Assert:
			EXPECT_NO_THROW(tree.wipe(keyIdentifier)) << keyIdentifier;
		}

		void AssertCanWipe(const BmKeyIdentifier& keyIdentifier) {
			// Arrange:
			TestContext context;

			// Act + Assert:
			AssertCanWipe(context.tree(), keyIdentifier);
		}

		void AssertCannotWipe(BmPrivateKeyTree& tree, const BmKeyIdentifier& keyIdentifier) {
			// Act + Assert:
			EXPECT_THROW(tree.wipe(keyIdentifier), catapult_invalid_argument) << keyIdentifier;
		}

		void AssertCannotWipe(const BmKeyIdentifier& keyIdentifier) {
			// Arrange:
			TestContext context;

			// Act + Assert:
			AssertCannotWipe(context.tree(), keyIdentifier);
		}
	}

	TEST(TEST_CLASS, CannotWipeWithKeyOutsideOfRange) {
		AssertCanWipe({ 10, Default_Options.Dilution - 1 });
		AssertCannotWipe({ 10, Default_Options.Dilution });
	}

	TEST(TEST_CLASS, CannotWipeWithKeyBelowStart) {
		AssertCannotWipe({ Start_Key.BatchId - 1, Start_Key.KeyId });
		AssertCannotWipe({ Start_Key.BatchId, Start_Key.KeyId - 1 });
	}

	TEST(TEST_CLASS, CannotWipeWithKeyAboveStart) {
		AssertCannotWipe({ End_Key.BatchId + 1, End_Key.KeyId });
		AssertCannotWipe({ End_Key.BatchId, End_Key.KeyId + 1});
	}

	TEST(TEST_CLASS, CannotWipeWithInvalidKey) {
		AssertCanWipe({ 10, BmKeyIdentifier::Invalid_Id });
		AssertCannotWipe({ BmKeyIdentifier::Invalid_Id, 3 });
	}

	TEST(TEST_CLASS, CannotWipeWithEarlierKey) {
		// Arrange:
		TestContext context;
		AssertCanWipe(context.tree(), { 10, 3 });

		// Act + Assert:
		AssertCannotWipe(context.tree(), { 10, 2 });
		AssertCannotWipe(context.tree(), { 9, 3 });

		// Sanity:
		AssertCanWipe(context.tree(), { 10, 3 });
	}

	// endregion

	// region roundtrip

	TEST(TEST_CLASS, RoundtripHasProperSignBehavior) {
		// Arrange:
		TestContext originalContext;
		BmKeyIdentifier earlierKeyIdentifier{ 9, 1 };
		BmKeyIdentifier keyIdentifier{ 9, 2 };
		BmKeyIdentifier laterKeyIdentifier{ 9, 3 };
		originalContext.tree().sign(keyIdentifier, test::GenerateRandomArray<10>());

		// Act: reload tree from storage
		TestContext context(originalContext);

		// Assert:
		test::AssertCannotSign(context.tree(), earlierKeyIdentifier);
		test::AssertCanSign(context.tree(), keyIdentifier);
		test::AssertCanSign(context.tree(), laterKeyIdentifier);

		// Sanity:
		test::AssertCannotSign(originalContext.tree(), earlierKeyIdentifier);
		test::AssertCanSign(originalContext.tree(), keyIdentifier);
		test::AssertCanSign(originalContext.tree(), laterKeyIdentifier);
	}

	TEST(TEST_CLASS, RoundtripHasProperWipeBehavior) {
		// Arrange:
		TestContext originalContext;
		originalContext.tree().sign({ 9, 3 }, test::GenerateRandomArray<10>());
		originalContext.tree().wipe({ 9, 1 });

		// Sanity:
		ASSERT_EQ(Full_L2_Size, originalContext.storage().buffer().size());
		test::AssertZeroedKeys(originalContext.storage().buffer(), L1_Payload_Start, 6, { 4, 5}, "L1");
		test::AssertZeroedKeys(originalContext.storage().buffer(), L2_Payload_Start, 7, { 5, 6 }, "L2");

		// Act: reload tree from storage
		TestContext context(originalContext);

		// Sanity:
		test::AssertCannotSign(context.tree(), { 9, 2 });
		test::AssertCanSign(context.tree(), { 9, 3 });

		// Act: wipe last used key
		context.tree().wipe({ 9, 3 });

		// Assert:
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 4, 5}, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 3, 4, 5, 6 }, "L2");
	}

	namespace {
		constexpr bool SingleLevelMatch(const BmTreeSignature& lhs, const BmTreeSignature& rhs) {
			return lhs.Root.ParentPublicKey == rhs.Root.ParentPublicKey
					&& lhs.Root.Signature == rhs.Root.Signature
					&& lhs.Top.ParentPublicKey == rhs.Top.ParentPublicKey;
		}
	}

	TEST(TEST_CLASS, RoundtripSingleLevelTree) {
		// Arrange:
		TestContext originalContext;

		// Act: reload single level tree from storage
		TestContext context(originalContext);

		// Assert: signatures are different, but all top level keys should match
		// '4' is used as KeyId, as it will be present in every BatchId
		EXPECT_EQ(originalContext.tree().rootPublicKey(), context.tree().rootPublicKey());
		test::AssertOptions(originalContext.tree().options(), context.tree().options());

		for (auto batchId = Start_Key.BatchId; batchId <= End_Key.BatchId; ++batchId) {
			auto messageBuffer = test::GenerateRandomArray<10>();
			auto originalSignature = originalContext.tree().sign({ batchId, 4 }, messageBuffer);
			auto signature = context.tree().sign({ batchId, 4 }, messageBuffer);

			EXPECT_TRUE(SingleLevelMatch(originalSignature, signature));
			EXPECT_NE(originalSignature, signature);
		}
	}

	TEST(TEST_CLASS, RoundtripMultiLevelTree) {
		// Arrange:
		TestContext originalContext;
		originalContext.tree().sign({ 9, 2 }, test::GenerateRandomArray<10>());

		// Act: reload tree from storage
		TestContext context(originalContext);

		// Assert:
		// - signatures on bottom level from both trees should match
		// - all signatures using keys (9, 3) - (9, 6) should match
		auto messageBuffer = test::GenerateRandomArray<10>();
		for (auto keyId = 3u; keyId < Default_Options.Dilution; ++keyId) {
			auto expectedSignature = originalContext.tree().sign({ 9, keyId }, messageBuffer);
			auto signature = context.tree().sign({ 9, keyId }, messageBuffer);

			EXPECT_EQ(expectedSignature, signature);
		}

		// - signature at different batch id will match only partially
		auto expectedSignature = originalContext.tree().sign({ 10, 0 }, messageBuffer);
		auto signature = context.tree().sign({ 10, 0 }, messageBuffer);

		EXPECT_NE(expectedSignature, signature);
		EXPECT_TRUE(SingleLevelMatch(expectedSignature, signature));
	}

	// endregion
}}
