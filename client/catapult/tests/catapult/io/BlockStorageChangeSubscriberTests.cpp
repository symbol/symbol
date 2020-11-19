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

#include "catapult/io/BlockStorageChangeSubscriber.h"
#include "catapult/io/BlockStorage.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS BlockStorageChangeSubscriberTests

	namespace {
		// region mocks

		class MockDroppingBlockStorage : public mocks::MockHeightOnlyBlockStorage {
		public:
			using mocks::MockHeightOnlyBlockStorage::MockHeightOnlyBlockStorage;

		public:
			std::vector<Height> Heights;

		public:
			void dropBlocksAfter(Height height) override {
				Heights.push_back(height);
			}
		};

		// endregion

		template<typename TTransactionStorage>
		class TestContext {
		public:
			explicit TestContext(Height storageHeight)
					: m_pStorage(std::make_unique<TTransactionStorage>(storageHeight))
					, m_pStorageRaw(m_pStorage.get())
					, m_pSubscriber(CreateBlockStorageChangeSubscriber(std::move(m_pStorage)))
			{}

		public:
			auto& storage() {
				return *m_pStorageRaw;
			}

			auto& subscriber() {
				return *m_pSubscriber;
			}

		private:
			std::unique_ptr<TTransactionStorage> m_pStorage; // notice that this is moved into m_pSubscriber
			TTransactionStorage* m_pStorageRaw;
			std::unique_ptr<BlockChangeSubscriber> m_pSubscriber;
		};
	}

	TEST(TEST_CLASS, NotifyBlockForwardsToStorage) {
		// Arrange:
		TestContext<mocks::MockSavingBlockStorage> context(Height(200));

		auto pBlock = test::GenerateEmptyRandomBlock();
		pBlock->Height = Height(987);
		auto blockElement = test::BlockToBlockElement(*pBlock, test::GenerateRandomByteArray<Hash256>());

		// Act:
		context.subscriber().notifyBlock(blockElement);

		// Assert: block was saved
		const auto& savedBlockElements = context.storage().savedBlockElements();
		ASSERT_EQ(1u, savedBlockElements.size());
		test::AssertEqual(blockElement, savedBlockElements[0]);
	}

	TEST(TEST_CLASS, NotifyDropBlocksAfterForwardsToStorage) {
		// Arrange:
		TestContext<MockDroppingBlockStorage> context(Height(200));

		// Act:
		context.subscriber().notifyDropBlocksAfter(Height(123));

		// Assert:
		auto expectedHeights = std::vector<Height>{ Height(123) };
		EXPECT_EQ(expectedHeights, context.storage().Heights);
	}
}}
