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

#include "catapult/crypto_voting/BmPrivateKeyTree.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/catapult/crypto_voting/test/BmTreeTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS BmPrivateKeyTreeTests

	namespace {
		constexpr auto Start_Key = BmKeyIdentifier{ 70 };
		constexpr auto End_Key = BmKeyIdentifier{ 79 };
		constexpr auto Num_Keys = End_Key.KeyId - Start_Key.KeyId + 1;
		constexpr BmOptions Default_Options{ Start_Key, End_Key };

		constexpr auto L1_Payload_Start = test::BmTreeSizes::CalculateLevelOnePayloadStart(0);
		constexpr auto Full_L1_Size = test::BmTreeSizes::CalculateFullLevelOneSize(Num_Keys);

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
		test::AssertZeroedKeys(storage.buffer(), L1_Payload_Start, Num_Keys, {}, "L1");
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
		AssertCanSign({ (Start_Key.KeyId + End_Key.KeyId) / 2 });
	}

	TEST(TEST_CLASS, CanSignWithEndKey) {
		AssertCanSign(End_Key);
	}

	TEST(TEST_CLASS, CanResignDifferentMessageWithSameKey) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		for (auto i = 0u; i < 5u; ++i)
			test::AssertCanSign(context.tree(), { 75 });
	}

	TEST(TEST_CLASS, CanResignSameMessageWithSameKey) {
		// Arrange:
		TestContext context;
		auto messageBuffer = test::GenerateRandomArray<10>();
		auto referenceSignature = context.tree().sign({ 75 }, messageBuffer);

		for (auto i = 0u; i < 5u; ++i) {
			// Act:
			auto signature = context.tree().sign({ 75 }, messageBuffer);

			// Assert:
			EXPECT_EQ(referenceSignature, signature) << i;
		}
	}

	TEST(TEST_CLASS, CanSignWithMultipleIncreasingValidKeys) {
		// Arrange:
		TestContext context;
		std::initializer_list<BmKeyIdentifier> keyIdentifiers{
			{ 71 },
			{ 73 },
			{ 74 },
			{ 78 }
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

	TEST(TEST_CLASS, CannotSignWithKeyBelowStart) {
		AssertCannotSign({ Start_Key.KeyId - 1 });
	}

	TEST(TEST_CLASS, CannotSignWithKeyAboveEnd) {
		AssertCannotSign({ End_Key.KeyId + 1 });
	}

	TEST(TEST_CLASS, CannotSignWithInvalidKey) {
		AssertCannotSign({ BmKeyIdentifier::Invalid_Id });
	}

	TEST(TEST_CLASS, CannotSignWithEarlierKey) {
		// Arrange:
		TestContext context;
		test::AssertCanSign(context.tree(), { 75 });

		// Act + Assert:
		test::AssertCannotSign(context.tree(), { 74 });
		test::AssertCannotSign(context.tree(), { 72 });

		// Sanity:
		test::AssertCanSign(context.tree(), { 75 });
	}

	// endregion

	// region wipe - success

	TEST(TEST_CLASS, CanWipeSingleKey) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 75 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 75 });

		// Assert:
		ASSERT_EQ(Full_L1_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5, 4 }, "L1");
	}

	TEST(TEST_CLASS, CanWipeConsecutiveKeys) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 75 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 76 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 75 });
		context.tree().wipe({ 76 });

		// Assert:
		ASSERT_EQ(Full_L1_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5, 4, 3 }, "L1");
	}

	TEST(TEST_CLASS, CanWipeNonConsecutiveKeys) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 75 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 78 }, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe({ 75 });
		context.tree().wipe({ 78 });

		// Assert:
		ASSERT_EQ(Full_L1_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5, 4, 3, 2, 1 }, "L1");
	}

	TEST(TEST_CLASS, CanWipeEndKey) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 75 }, test::GenerateRandomArray<10>());
		context.tree().sign(End_Key, test::GenerateRandomArray<10>());

		// Act:
		context.tree().wipe(End_Key);

		// Assert:
		ASSERT_EQ(Full_L1_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }, "L1");
	}

	TEST(TEST_CLASS, CanWipeSingleKeyMultipleTimes) {
		// Arrange:
		TestContext context;
		context.tree().sign({ 75 }, test::GenerateRandomArray<10>());

		// Act:
		for (auto i = 0u; i < 5; ++ i)
			context.tree().wipe({ 75 });

		// Assert:
		ASSERT_EQ(Full_L1_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5, 4 }, "L1");
	}

	// endregion

	// region wipe - failure

	namespace {
		void AssertCanWipe(BmPrivateKeyTree& tree, const BmKeyIdentifier& keyIdentifier) {
			// Act + Assert:
			EXPECT_NO_THROW(tree.wipe(keyIdentifier)) << keyIdentifier;
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

	TEST(TEST_CLASS, CannotWipeWithKeyBelowStart) {
		AssertCannotWipe({ Start_Key.KeyId - 1 });
	}

	TEST(TEST_CLASS, CannotWipeWithKeyAboveEnd) {
		AssertCannotWipe({ End_Key.KeyId + 1 });
	}

	TEST(TEST_CLASS, CannotWipeWithInvalidKey) {
		AssertCannotWipe({ BmKeyIdentifier::Invalid_Id });
	}

	TEST(TEST_CLASS, CannotWipeWithEarlierKey) {
		// Arrange:
		TestContext context;
		AssertCanWipe(context.tree(), { 75 });

		// Act + Assert:
		AssertCannotWipe(context.tree(), { 74 });
		AssertCannotWipe(context.tree(), { 72 });

		// Sanity:
		AssertCanWipe(context.tree(), { 75 });
	}

	// endregion

	// region roundtrip

	TEST(TEST_CLASS, RoundtripHasProperSignBehavior) {
		// Arrange:
		TestContext originalContext;
		BmKeyIdentifier earlierKeyIdentifier{ 74 };
		BmKeyIdentifier keyIdentifier{ 75 };
		BmKeyIdentifier laterKeyIdentifier{ 76 };
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
		originalContext.tree().sign({ 76 }, test::GenerateRandomArray<10>());
		originalContext.tree().wipe({ 74 });

		// Sanity:
		ASSERT_EQ(Full_L1_Size, originalContext.storage().buffer().size());
		test::AssertZeroedKeys(originalContext.storage().buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5 }, "L1");

		// Act: reload tree from storage
		TestContext context(originalContext);

		// Sanity:
		test::AssertCannotSign(context.tree(), { 75 });
		test::AssertCanSign(context.tree(), { 76 });

		// Act: wipe last used key
		context.tree().wipe({ 76 });

		// Assert:
		ASSERT_EQ(Full_L1_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5, 4, 3 }, "L1");
	}

	TEST(TEST_CLASS, RoundtripSingleLevelTree) {
		// Arrange:
		TestContext originalContext;

		// Act: reload single level tree from storage
		TestContext context(originalContext);

		// Assert: properties are equal
		EXPECT_EQ(originalContext.tree().rootPublicKey(), context.tree().rootPublicKey());
		test::AssertOptions(originalContext.tree().options(), context.tree().options());

		// - signatures are equal
		for (auto keyId = Start_Key.KeyId; keyId <= End_Key.KeyId; ++keyId) {
			auto messageBuffer = test::GenerateRandomArray<10>();
			auto originalSignature = originalContext.tree().sign({ keyId }, messageBuffer);
			auto signature = context.tree().sign({ keyId }, messageBuffer);

			EXPECT_EQ(originalSignature, signature) << "keyId " << keyId;
		}
	}

	// endregion
}}
