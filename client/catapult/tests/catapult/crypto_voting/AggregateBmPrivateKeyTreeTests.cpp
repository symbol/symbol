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

#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/catapult/crypto_voting/test/BmTreeTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS AggregateBmPrivateKeyTreeTests

	namespace {
		constexpr auto Start_Key = BmKeyIdentifier{ 8, 4 };
		constexpr auto End_Key = BmKeyIdentifier{ 13, 5 };
		constexpr BmOptions Default_Options{ 7, Start_Key, End_Key };

		constexpr auto L1_Payload_Start = test::BmTreeSizes::CalculateLevelOnePayloadStart(0);
		constexpr auto L2_Payload_Start = test::BmTreeSizes::CalculateLevelTwoPayloadStart(6);
		constexpr auto Full_L1_Size = test::BmTreeSizes::CalculateFullLevelOneSize(6);
		constexpr auto Full_L2_Size = test::BmTreeSizes::CalculateFullLevelTwoSize(6, 7);

		auto GenerateKeyPair() {
			return test::GenerateKeyPair();
		}

		// region test context

		class TestContext {
		public:
			TestContext() : m_tree(AggregateBmPrivateKeyTree::Create(GenerateKeyPair(), m_storage, Default_Options))
			{}

			TestContext(TestContext& originalContext)
					: m_tree(AggregateBmPrivateKeyTree::FromStream(CopyInto(originalContext.m_storage, m_storage)))
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
			AggregateBmPrivateKeyTree m_tree;
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
		auto tree = AggregateBmPrivateKeyTree::Create(std::move(rootKeyPair), storage, Default_Options);

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
			auto originalTree = AggregateBmPrivateKeyTree::Create(std::move(rootKeyPair), storage, Default_Options);
			storage.seek(0);
		}

		// Act:
		auto tree = AggregateBmPrivateKeyTree::FromStream(storage);

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

	TEST(TEST_CLASS, CanResignWithSameKey) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		for (auto i = 0u; i < 5u; ++i)
			test::AssertCanSign(context.tree(), { 10, 3 });
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

	// region wipe - saving

	TEST(TEST_CLASS, SignCreatesSubLevelsAndAutomaticallyWipesPreviousKeysInSameBatch) {
		// Arrange:
		TestContext context;

		// Act:
		context.tree().sign({ 9, 3 }, test::GenerateRandomArray<10>());

		// Assert: (8,) (9,) L1 and (9,0) (9,1) (9,2) L2 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, { 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, SignCreatesSubLevelsAndAutomaticallyWipesPreviousKeysInDifferentBatch) {
		// Arrange:
		TestContext context;

		// Act:
		context.tree().sign({ 9, 3 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 12, 0 }, test::GenerateRandomArray<10>());

		// Assert: (8,) (9,) (10,) (11,) (12,) are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage().buffer().size());
		test::AssertZeroedKeys(context.storage().buffer(), L1_Payload_Start, 6, { 1, 2, 3, 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage().buffer(), L2_Payload_Start, 7, {}, "L2");
	}

	// endregion
}}
