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

#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/catapult/crypto_voting/test/BmTreeTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS AggregateBmPrivateKeyTreeTests

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
		TestContext context({ { { 8 }, { 14 } }, { { 15 }, { 27 } } });

		// Sanity:
		EXPECT_EQ(context.publicKey(0), context.tree().rootPublicKey());
		test::AssertOptions({ { 8 }, { 14 } }, context.tree().options());

		// Act: advance to next tree
		context.tree().canSign({ 19 });

		// Assert:
		EXPECT_EQ(context.publicKey(1), context.tree().rootPublicKey());
		test::AssertOptions({ { 15 }, { 27 } }, context.tree().options());

		// - (storage) used tree is cleared
		ASSERT_EQ(test::BmTreeSizes::CalculateFullLevelOneSize(7), context.storage(0).buffer().size());
		test::AssertZeroedKeys(context.storage(0).buffer(), L1_Payload_Start, 7, { 6, 5, 4, 3, 2, 1, 0 }, "L1");
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

	// region canSign / sign - factory

	namespace {
		template<typename TAction>
		void RunMultiTreeFactoryTest(TAction action) {
			// Arrange:
			TestContext context({
				{ { 12 }, { 18 } },
				{ { 19 }, { 30 } },
				{ { 45 }, { 60 } }
			});

			// Act + Assert:
			action(context);
		}
	}

	TEST(TEST_CLASS, CanSignWithAnyTreeReturnedByFactory) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			test::AssertCanSign(context.tree(), { 16 }); // tree 1
			test::AssertCanSign(context.tree(), { 19 }); // tree 2
			test::AssertCanSign(context.tree(), { 25 });
			test::AssertCanSign(context.tree(), { 30 });
			test::AssertCanSign(context.tree(), { 55 }); // tree 3
		});
	}

	TEST(TEST_CLASS, CannotSignWithinAnyGapsInTreesReturnedByFactory) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			test::AssertCannotSign(context.tree(), { 11 }); // before first
			test::AssertCannotSign(context.tree(), { 35 }); // gap
			test::AssertCannotSign(context.tree(), { 61 }); // after last
		});
	}

	TEST(TEST_CLASS, CanSignBeforeAndAfterGapInTreesReturnedByFactory) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			test::AssertCanSign(context.tree(), { 30 }); // before gap
			test::AssertCannotSign(context.tree(), { 35 }); // gap
			test::AssertCanSign(context.tree(), { 45 }); // after gap
		});
	}

	TEST(TEST_CLASS, CannotSignWhenTreesReturnedByFactoryOverlap_CanSign) {
		// Arrange:
		for (auto overlapBatchId : std::initializer_list<uint64_t>{ 15, 18 }) {
			TestContext context({ { { 12 }, { 18 } }, { { overlapBatchId }, { 30 } } });

			// Sanity:
			test::AssertCanSign(context.tree(), { 16 });

			// Act + Assert:
			EXPECT_THROW(context.tree().canSign({ 20 }), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, CannotSignWhenTreesReturnedByFactoryOverlap_Sign) {
		// Arrange:
		for (auto overlapBatchId : std::initializer_list<uint64_t>{ 15, 18 }) {
			TestContext context({ { { 12 }, { 18 } }, { { overlapBatchId }, { 30 } } });

			// Sanity:
			test::AssertCanSign(context.tree(), { 16 });

			// Act + Assert:
			EXPECT_THROW(context.tree().sign({ 20 }, test::GenerateRandomArray<10>()), catapult_runtime_error);
		}
	}

	// endregion

	// region wipe - saving

	TEST(TEST_CLASS, SignAutomaticallyWipesPreviousKeys) {
		// Arrange:
		TestContext context;

		// Act:
		context.tree().sign({ 75 }, test::GenerateRandomArray<10>());

		// Assert:
		ASSERT_EQ(Full_L1_Size, context.storage(0).buffer().size());
		test::AssertZeroedKeys(context.storage(0).buffer(), L1_Payload_Start, Num_Keys, { 9, 8, 7, 6, 5 }, "L1");
	}

	TEST(TEST_CLASS, SignCreatesSubLevelsAndAutomaticallyWipesPreviousKeysInDifferentTree) {
		// Arrange:
		RunMultiTreeFactoryTest([](auto& context) {
			// Act + Assert:
			context.tree().sign({ 16 }, test::GenerateRandomArray<10>());
			context.tree().sign({ 23 }, test::GenerateRandomArray<10>());

			// Assert: everything is cleared in first tree
			ASSERT_EQ(test::BmTreeSizes::CalculateFullLevelOneSize(7), context.storage(0).buffer().size());
			test::AssertZeroedKeys(context.storage(0).buffer(), L1_Payload_Start, 7, { 6, 5, 4, 3, 2, 1, 0 }, "L1[0]");

			// - (19, 22) keys are cleared in second tree
			ASSERT_EQ(test::BmTreeSizes::CalculateFullLevelOneSize(12), context.storage(1).buffer().size());
			test::AssertZeroedKeys(context.storage(1).buffer(), L1_Payload_Start, 12, { 11, 10, 9, 8 }, "L1[1]");
		});
	}

	// endregion
}}
