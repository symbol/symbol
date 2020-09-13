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
			return VotingKeyPair::FromPrivate(VotingPrivateKey::Generate(test::RandomByte));
		}

		// region test context

		class TestContext {
		public:
			TestContext() : TestContext({ Default_Options })
			{}

			explicit TestContext(const std::vector<BmOptions>& options)
					: m_nextStorageIndex(0)
					, m_storages(options.size())
					, m_tree([this, options]() {
						if (m_nextStorageIndex >= options.size())
							return std::unique_ptr<BmPrivateKeyTree>();

						auto keyPair = GenerateKeyPair();
						m_publicKeys.push_back(keyPair.publicKey());
						auto tree = BmPrivateKeyTree::Create(
								std::move(keyPair),
								m_storages[m_nextStorageIndex],
								options[m_nextStorageIndex]);
						++m_nextStorageIndex;
						return std::make_unique<BmPrivateKeyTree>(std::move(tree));
					})
			{}

		public:
			const auto& publicKey(size_t index) {
				return m_publicKeys[index];
			}

			auto& storage(size_t index) {
				return m_storages[index];
			}

			auto& tree() {
				return m_tree;
			}

		private:
			size_t m_nextStorageIndex;
			std::vector<VotingKey> m_publicKeys;
			std::vector<mocks::MockSeekableMemoryStream> m_storages;
			AggregateBmPrivateKeyTree m_tree;
		};

		// endregion
	}

	// region ctor / properties

	TEST(TEST_CLASS, CanCreateEmptyTree) {
		// Arrange:
		auto rootKeyPair = GenerateKeyPair();
		auto expectedPublicKey = rootKeyPair.publicKey();
		mocks::MockSeekableMemoryStream storage;

		// Act:
		auto tree = AggregateBmPrivateKeyTree([&rootKeyPair, &storage]() {
			return std::make_unique<BmPrivateKeyTree>(BmPrivateKeyTree::Create(std::move(rootKeyPair), storage, Default_Options));
		});

		// Assert:
		EXPECT_EQ(expectedPublicKey, tree.rootPublicKey());
		test::AssertOptions(Default_Options, tree.options());

		// - (storage) all level one keys are created upon initial creation
		ASSERT_EQ(Full_L1_Size, storage.buffer().size());
		test::AssertZeroedKeys(storage.buffer(), L1_Payload_Start, 6, {}, "L1");
	}

	TEST(TEST_CLASS, PropertiesAreUpdatedWhenTreeIsAdvanced) {
		// Arrange:
		TestContext context({ { 7, { 8, 4 }, { 13, 5 } }, { 7, { 13, 6 }, { 17, 5 } } });

		// Sanity:
		EXPECT_EQ(context.publicKey(0), context.tree().rootPublicKey());
		test::AssertOptions({ 7, { 8, 4 }, { 13, 5 } }, context.tree().options());

		// Act: advance to next tree
		context.tree().canSign({ 14, 2 });

		// Assert:
		EXPECT_EQ(context.publicKey(1), context.tree().rootPublicKey());
		test::AssertOptions({ 7, { 13, 6 }, { 17, 5 } }, context.tree().options());

		// - (storage) used tree is cleared
		ASSERT_EQ(Full_L1_Size, context.storage(0).buffer().size());
		test::AssertZeroedKeys(context.storage(0).buffer(), L1_Payload_Start, 6, { 0, 1, 2, 3, 4, 5 }, "L1");
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

	// region canSign / sign - factory

	namespace {
		template<typename TAction>
		void RunMultiTreeFactoryTest(TAction action) {
			// Arrange:
			TestContext context({
				{ 7, { 8, 4 }, { 13, 5 } },
				{ 7, { 13, 6 }, { 17, 5 } },
				{ 7, { 20, 2 }, { 25, 3 } }
			});

			// Act + Assert:
			action(context);
		}
	}

	TEST(TEST_CLASS, CanSignWithAnyTreeReturnedByFactory) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			test::AssertCanSign(context.tree(), { 10, 3 }); // tree 1
			test::AssertCanSign(context.tree(), { 13, 6 }); // tree 2
			test::AssertCanSign(context.tree(), { 15, 5 });
			test::AssertCanSign(context.tree(), { 17, 5 });
			test::AssertCanSign(context.tree(), { 21, 2 }); // tree 3
		});
	}

	TEST(TEST_CLASS, CannotSignWithinAnyGapsInTreesReturnedByFactory) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			test::AssertCannotSign(context.tree(), { 8, 3 }); // before first
			test::AssertCannotSign(context.tree(), { 18, 2 }); // gap
			test::AssertCannotSign(context.tree(), { 25, 4 }); // after last
		});
	}

	TEST(TEST_CLASS, CanSignBeforeAndAfterGapInTreesReturnedByFactory) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			test::AssertCanSign(context.tree(), { 10, 3 }); // before gap
			test::AssertCannotSign(context.tree(), { 18, 2 }); // gap
			test::AssertCanSign(context.tree(), { 21, 2 }); // after gap
		});
	}

	TEST(TEST_CLASS, CannotSignWhenTreesReturnedByFactoryOverlap_CanSign) {
		// Arrange:
		for (auto overlapBatchId : std::initializer_list<uint64_t>{ 12, 13 }) {
			TestContext context({ { 7, { 8, 4 }, { 13, 5 } }, { 7, { overlapBatchId, 5 }, { 17, 5 } } });

			// Sanity:
			test::AssertCanSign(context.tree(), { 10, 3 });

			// Act + Assert:
			EXPECT_THROW(context.tree().canSign({ 14, 1 }), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, CannotSignWhenTreesReturnedByFactoryOverlap_Sign) {
		// Arrange:
		for (auto overlapBatchId : std::initializer_list<uint64_t>{ 12, 13 }) {
			TestContext context({ { 7, { 8, 4 }, { 13, 5 } }, { 7, { overlapBatchId, 5 }, { 17, 5 } } });

			// Sanity:
			test::AssertCanSign(context.tree(), { 10, 3 });

			// Act + Assert:
			EXPECT_THROW(context.tree().sign({ 14, 1 }, test::GenerateRandomArray<10>()), catapult_runtime_error);
		}
	}

	// endregion

	// region wipe - saving

	TEST(TEST_CLASS, SignCreatesSubLevelsAndAutomaticallyWipesPreviousKeysInSameBatch) {
		// Arrange:
		TestContext context;

		// Act:
		context.tree().sign({ 9, 3 }, test::GenerateRandomArray<10>());

		// Assert: (8,) (9,) L1 and (9,0) (9,1) (9,2) L2 are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage(0).buffer().size());
		test::AssertZeroedKeys(context.storage(0).buffer(), L1_Payload_Start, 6, { 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage(0).buffer(), L2_Payload_Start, 7, { 4, 5, 6 }, "L2");
	}

	TEST(TEST_CLASS, SignCreatesSubLevelsAndAutomaticallyWipesPreviousKeysInDifferentBatch) {
		// Arrange:
		TestContext context;

		// Act:
		context.tree().sign({ 9, 3 }, test::GenerateRandomArray<10>());
		context.tree().sign({ 12, 0 }, test::GenerateRandomArray<10>());

		// Assert: (8,) (9,) (10,) (11,) (12,) are cleared (keys are in reverse order)
		ASSERT_EQ(Full_L2_Size, context.storage(0).buffer().size());
		test::AssertZeroedKeys(context.storage(0).buffer(), L1_Payload_Start, 6, { 1, 2, 3, 4, 5 }, "L1");
		test::AssertZeroedKeys(context.storage(0).buffer(), L2_Payload_Start, 7, {}, "L2");
	}

	TEST(TEST_CLASS, SignCreatesSubLevelsAndAutomaticallyWipesPreviousKeysInDifferentTree) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			context.tree().sign({ 10, 3 }, test::GenerateRandomArray<10>());
			context.tree().sign({ 14, 2 }, test::GenerateRandomArray<10>());

			// Assert: everything is cleared except (13, 6), an unused key above end of first tree (keys are in reverse order)
			ASSERT_EQ(Full_L2_Size, context.storage(0).buffer().size());
			test::AssertZeroedKeys(context.storage(0).buffer(), L1_Payload_Start, 6, { 0, 1, 2, 3, 4, 5 }, "L1[0]");
			test::AssertZeroedKeys(context.storage(0).buffer(), L2_Payload_Start, 7, { 1, 2, 3, 4, 5, 6 }, "L2[0]");

			// - (13,) (14,) L1 and (14,0) (14,1) L2 are cleared (keys are in reverse order)
			ASSERT_EQ(test::BmTreeSizes::CalculateFullLevelTwoSize(5, 7), context.storage(1).buffer().size());
			test::AssertZeroedKeys(context.storage(1).buffer(), L1_Payload_Start, 5, { 3, 4 }, "L1[1]");
			test::AssertZeroedKeys(context.storage(1).buffer(), test::BmTreeSizes::CalculateLevelTwoPayloadStart(5), 7, { 5, 6 }, "L2[1]");
		});
	}

	// endregion
}}
