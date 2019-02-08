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

#include "mongo/src/MongoBlockStorageUtils.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/io/BlockStorage.h"
#include "catapult/model/Address.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/NotificationSubscriber.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS MongoBlockStorageUtilsTests

	namespace {
		// region mocks

		class MockNotificationPublisher : public model::NotificationPublisher {
		public:
			void publish(const model::WeakEntityInfo& entityInfo, model::NotificationSubscriber& sub) const override {
				// just publish the signer
				sub.notify(model::AccountPublicKeyNotification(entityInfo.entity().Signer));
			}
		};

		class MockLoadingBlockStorage : public mocks::MockHeightOnlyBlockStorage {
		public:
			MockLoadingBlockStorage(Height chainHeight)
					: mocks::MockHeightOnlyBlockStorage(chainHeight)
					, m_pBlock(test::GenerateBlockWithTransactions(3))
					, m_blockElement(test::BlockToBlockElement(*m_pBlock, test::GenerateRandomData<Hash256_Size>())) {
				// simulate file storage by clearing all extracted addresses
				for (auto& transactionElement : m_blockElement.Transactions)
					transactionElement.OptionalExtractedAddresses = nullptr;
			}

		public:
			mutable std::vector<Height> LoadedBlockElementHeights;
			mutable std::vector<model::BlockElement> LoadedBlockElements;

		public:
			std::shared_ptr<const model::BlockElement> loadBlockElement(Height height) const override {
				LoadedBlockElementHeights.push_back(height);
				LoadedBlockElements.push_back(m_blockElement);

				// return a non-owning shared_ptr referencing the m_blockElement member
				return std::shared_ptr<const model::BlockElement>(&m_blockElement, [](const auto*) {});
			}

		private:
			std::unique_ptr<model::Block> m_pBlock;
			model::BlockElement m_blockElement;
		};

		// endregion

		template<typename TTransactionStorage1, typename TTransactionStorage2 = TTransactionStorage1>
		class TestContext {
		public:
			TestContext(Height primaryStorageHeight, Height secondaryStorageHeight)
					: m_pPrimaryStorage(std::make_unique<TTransactionStorage1>(primaryStorageHeight))
					, m_pSecondaryStorage(std::make_unique<TTransactionStorage2>(secondaryStorageHeight))
			{}

		public:
			auto& primaryStorage() {
				return *m_pPrimaryStorage;
			}

			auto& secondaryStorage() {
				return *m_pSecondaryStorage;
			}

		public:
			void prepare() {
				PrepareMongoBlockStorage(primaryStorage(), secondaryStorage(), MockNotificationPublisher());
			}

		private:
			std::unique_ptr<TTransactionStorage1> m_pPrimaryStorage;
			std::unique_ptr<TTransactionStorage2> m_pSecondaryStorage;
		};
	}

	TEST(TEST_CLASS, PrepareMongoBlockStorageSavesSecondaryStorageNemesisInPrimaryStorage) {
		// Arrange:
		TestContext<mocks::MockSavingBlockStorage, MockLoadingBlockStorage> context(Height(0), Height(4));

		// Act:
		context.prepare();

		// Assert:
		EXPECT_EQ(std::vector<Height>({ Height(1) }), context.secondaryStorage().LoadedBlockElementHeights);

		// - the block loaded by the secondary storage was saved into the primary storage
		ASSERT_EQ(1u, context.primaryStorage().savedBlockElements().size());
		ASSERT_EQ(1u, context.secondaryStorage().LoadedBlockElementHeights.size());

		const auto& loadedBlockElement = context.secondaryStorage().LoadedBlockElements[0];
		const auto& savedBlockElement = context.primaryStorage().savedBlockElements()[0];
		test::AssertEqual(loadedBlockElement, savedBlockElement);

		// - the loaded block does not have any extracted addresses
		EXPECT_EQ(3u, loadedBlockElement.Transactions.size());
		for (const auto& transactionElement : loadedBlockElement.Transactions)
			EXPECT_FALSE(!!transactionElement.OptionalExtractedAddresses);

		// - the saved block has extracted addresses
		EXPECT_EQ(3u, savedBlockElement.Transactions.size());
		for (const auto& transactionElement : savedBlockElement.Transactions) {
			ASSERT_TRUE(!!transactionElement.OptionalExtractedAddresses);
			ASSERT_EQ(1u, transactionElement.OptionalExtractedAddresses->size());

			const auto& transaction = transactionElement.Transaction;
			auto signerAddress = extensions::CopyToUnresolvedAddress(model::PublicKeyToAddress(transaction.Signer, transaction.Network()));
			EXPECT_EQ(signerAddress, *transactionElement.OptionalExtractedAddresses->cbegin());
		}
	}
}}
