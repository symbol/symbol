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

#include "catapult/observers/RentalFeeObserver.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS RentalFeeObserverTests

	namespace {
		using ObserverTestContext = test::ObserverTestContextT<test::CoreSystemCacheFactory>;

		constexpr auto Mock_Notification = static_cast<model::NotificationType>(0xFFFF'FFFF);
		constexpr auto Default_Receipt_Type = static_cast<model::ReceiptType>(0x1234);

		struct MockRentalFeeNotification : public model::BalanceTransferNotification {
		public:
			MockRentalFeeNotification(
					const Address& sender,
					const UnresolvedAddress& recipient,
					UnresolvedMosaicId mosaicId,
					catapult::Amount amount)
					: BalanceTransferNotification(sender, recipient, mosaicId, amount) {
				// override type
				Type = Mock_Notification;
			}
		};

		auto CreateMockRentalFeeObserver() {
			return observers::CreateRentalFeeObserver<MockRentalFeeNotification>("Test", Default_Receipt_Type);
		}

		template<typename TAssert>
		void RunObserverTest(observers::NotifyMode mode, TAssert assertContext) {
			// Arrange: create observer and notification
			auto pObserver = CreateMockRentalFeeObserver();
			auto sender = test::GenerateRandomByteArray<Address>();
			auto recipient = test::GenerateRandomByteArray<Address>();
			MockRentalFeeNotification notification(sender, test::UnresolveXor(recipient), test::UnresolveXor(MosaicId(345)), Amount(123));

			ObserverTestContext context(mode, Height(888));
			context.state().DynamicFeeMultiplier = BlockFeeMultiplier(999);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert:
			assertContext(context, sender, recipient);
		}
	}

	TEST(TEST_CLASS, CanCreateObserver) {
		// Act:
		auto pObserver = CreateMockRentalFeeObserver();

		// Assert:
		EXPECT_EQ("TestRentalFeeObserver", pObserver->name());
	}

	TEST(TEST_CLASS, AddsReceiptOnCommit) {
		RunObserverTest(observers::NotifyMode::Commit, [](auto& context, const auto& sender, const auto& recipient) {
			// Assert:
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(1u, pStatement->TransactionStatements.size());
			const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
			ASSERT_EQ(1u, receiptPair.second.size());

			const auto& receipt = static_cast<const model::BalanceTransferReceipt&>(receiptPair.second.receiptAt(0));
			ASSERT_EQ(sizeof(model::BalanceTransferReceipt), receipt.Size);
			EXPECT_EQ(1u, receipt.Version);
			EXPECT_EQ(Default_Receipt_Type, receipt.Type);
			EXPECT_EQ(MosaicId(345), receipt.Mosaic.MosaicId);
			EXPECT_EQ(Amount(123 * 999), receipt.Mosaic.Amount);
			EXPECT_EQ(sender, receipt.SenderAddress);
			EXPECT_EQ(recipient, receipt.RecipientAddress);
		});
	}

	TEST(TEST_CLASS, DoesNotAddReceiptOnRollback) {
		RunObserverTest(observers::NotifyMode::Rollback, [](auto& context, const auto&, const auto&) {
			// Assert:
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(0u, pStatement->TransactionStatements.size());
		});
	}
}}
