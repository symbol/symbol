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

#include "filespooling/src/FileUtChangeStorage.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "filespooling/tests/test/FileTransactionsChangeStorageContext.h"
#include "filespooling/tests/test/StorageTransactionInfoTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"

namespace catapult { namespace filespooling {

#define TEST_CLASS FileUtChangeStorageTests

	namespace {
		// region test context

		struct SubscriberTraits {
			using SubscriberType = cache::UtChangeSubscriber;
			using OperationType = subscribers::UtChangeOperationType;

			static constexpr auto Create = CreateFileUtChangeStorage;
		};

		class FileUtChangeStorageContext : public test::FileTransactionsChangeStorageContext<SubscriberTraits> {};

		// endregion
	}

	TEST(TEST_CLASS, NotifyAddsSavesNotifications) {
		// Arrange:
		FileUtChangeStorageContext context;
		auto transactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(3);

		// Act:
		context.subscriber().notifyAdds(transactionInfos);

		// Assert:
		context.assertFileContents({ { subscribers::UtChangeOperationType::Add, std::cref(transactionInfos) } });
		context.assertNumFlushes(0);
	}

	TEST(TEST_CLASS, NotifyRemovesSavesNotifications) {
		// Arrange:
		FileUtChangeStorageContext context;
		auto transactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(3);

		// Act:
		context.subscriber().notifyRemoves(transactionInfos);

		// Assert:
		context.assertFileContents({ { subscribers::UtChangeOperationType::Remove, std::cref(transactionInfos) } });
		context.assertNumFlushes(0);
	}

	TEST(TEST_CLASS, BothNotifyAddsAndNotifyRemovesSaveNotifications) {
		// Arrange:
		FileUtChangeStorageContext context;
		auto addedTransactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(3);
		auto removedTransactionInfos = test::CreateTransactionInfosSetWithOptionalAddresses(4);

		// Act:
		context.subscriber().notifyAdds(addedTransactionInfos);
		context.subscriber().notifyRemoves(removedTransactionInfos);

		// Assert:
		context.assertFileContents({
			{ subscribers::UtChangeOperationType::Add, std::cref(addedTransactionInfos) },
			{ subscribers::UtChangeOperationType::Remove, std::cref(removedTransactionInfos) }
		});
		context.assertNumFlushes(0);
	}

	TEST(TEST_CLASS, FlushFlushesUnderlyingStream) {
		// Arrange:
		FileUtChangeStorageContext context;

		// Act:
		context.subscriber().flush();

		// Assert:
		context.assertEmptyBuffer();
		context.assertNumFlushes(1);
	}
}}
