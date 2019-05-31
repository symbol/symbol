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

#include "filespooling/src/FilePtChangeStorage.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "filespooling/tests/test/FileTransactionsChangeStorageContext.h"
#include "filespooling/tests/test/StorageTransactionInfoTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"

namespace catapult { namespace filespooling {

#define TEST_CLASS FilePtChangeStorageTests

	namespace {
		// region test context

		struct SubscriberTraits {
			using SubscriberType = cache::PtChangeSubscriber;
			using OperationType = subscribers::PtChangeOperationType;

			static constexpr auto Create = CreateFilePtChangeStorage;
		};

		class FilePtChangeStorageContext : public test::FileTransactionsChangeStorageContext<SubscriberTraits> {
		public:
			void assertCosignature(
					const Key& expectedKey,
					const Signature& expectedSignature,
					const model::TransactionInfo& expectedTransactionInfo) {
				auto inputStream = createInputStream();
				Key key;
				Signature signature;
				model::TransactionInfo transactionInfo;
				auto operationType = static_cast<OperationType>(io::Read8(inputStream));
				inputStream.read(key);
				inputStream.read(signature);
				io::ReadTransactionInfo(inputStream, transactionInfo);

				EXPECT_EQ(subscribers::PtChangeOperationType::Add_Cosignature, operationType);
				EXPECT_EQ(expectedKey, key);
				EXPECT_EQ(expectedSignature, signature);

				test::AssertEqual(expectedTransactionInfo, transactionInfo);
			}
		};

		// endregion
	}

	TEST(TEST_CLASS, NotifyAddPartialsSavesNotifications) {
		// Arrange:
		FilePtChangeStorageContext context;
		auto transactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(3);

		// Act:
		context.subscriber().notifyAddPartials(transactionInfos);

		// Assert:
		context.assertFileContents({ { subscribers::PtChangeOperationType::Add_Partials, std::cref(transactionInfos) } });
		context.assertNumFlushes(0);
	}

	TEST(TEST_CLASS, NotifyRemovePartialsSavesNotifications) {
		// Arrange:
		FilePtChangeStorageContext context;
		auto transactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(3);

		// Act:
		context.subscriber().notifyRemovePartials(transactionInfos);

		// Assert:
		context.assertFileContents({ { subscribers::PtChangeOperationType::Remove_Partials, std::cref(transactionInfos) } });
		context.assertNumFlushes(0);
	}

	TEST(TEST_CLASS, BothNotifyAddPartialsAndNotifyRemovePartialsSaveNotifications) {
		// Arrange:
		FilePtChangeStorageContext context;
		auto addedTransactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(3);
		auto removedTransactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(4);

		// Act:
		context.subscriber().notifyAddPartials(addedTransactionInfos);
		context.subscriber().notifyRemovePartials(removedTransactionInfos);

		// Assert:
		context.assertFileContents({
			{ subscribers::PtChangeOperationType::Add_Partials, std::cref(addedTransactionInfos) },
			{ subscribers::PtChangeOperationType::Remove_Partials, std::cref(removedTransactionInfos) }
		});
		context.assertNumFlushes(0);
	}

	TEST(TEST_CLASS, NotifyAddCosignatureSavesCosignature) {
		// Arrange:
		FilePtChangeStorageContext context;
		auto key = test::GenerateRandomByteArray<Key>();
		auto signature = test::GenerateRandomByteArray<Signature>();
		auto transactionInfo = test::CreateRandomTransactionInfo();
		transactionInfo.OptionalExtractedAddresses = test::GenerateRandomUnresolvedAddressSetPointer(3);

		// Act:
		context.subscriber().notifyAddCosignature(transactionInfo, key, signature);

		// Assert:
		context.assertCosignature(key, signature, transactionInfo);
		context.assertNumFlushes(0);
	}

	TEST(TEST_CLASS, FlushFlushesUnderlyingStream) {
		// Arrange:
		FilePtChangeStorageContext context;

		// Act:
		context.subscriber().flush();

		// Assert:
		context.assertEmptyBuffer();
		context.assertNumFlushes(1);
	}
}}
