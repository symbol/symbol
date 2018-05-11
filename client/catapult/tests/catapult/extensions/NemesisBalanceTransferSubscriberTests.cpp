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

#include "catapult/extensions/NemesisBalanceTransferSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS NemesisBalanceTransferSubscriberTests

	namespace {
		class TestContext {
		public:
			TestContext()
					: m_nemesisPublicKey(test::GenerateRandomData<Key_Size>())
					, m_subscriber(m_nemesisPublicKey)
			{}

		public:
			const auto& outflows() {
				return m_subscriber.outflows();
			}

		public:
			void notify(MosaicId mosaicId, Amount amount) {
				notify(m_nemesisPublicKey, mosaicId, amount);
			}

			void notify(const Key& sender, MosaicId mosaicId, Amount amount) {
				notify(model::BalanceTransferNotification(sender, test::GenerateRandomData<Address_Decoded_Size>(), mosaicId, amount));
			}

			void notify(const model::Notification& notification) {
				m_subscriber.notify(notification);
			}

		private:
			Key m_nemesisPublicKey;
			NemesisBalanceTransferSubscriber m_subscriber;
		};
	}

	TEST(TEST_CLASS, InitiallyHasNoOutflows) {
		// Act:
		TestContext context;

		// Assert:
		const auto& outflows = context.outflows();
		EXPECT_TRUE(outflows.empty());
	}

	TEST(TEST_CLASS, IgnoresNonTransferNotifications) {
		// Arrange:
		TestContext context;

		// Act:
		context.notify(model::AccountPublicKeyNotification(test::GenerateRandomData<Key_Size>()));

		// Assert:
		const auto& outflows = context.outflows();
		EXPECT_TRUE(outflows.empty());
	}

	TEST(TEST_CLASS, FailsWhenObservedTransferIsFromNonNemesisAccount) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		EXPECT_THROW(context.notify(test::GenerateRandomData<Key_Size>(), MosaicId(123), Amount(111)), catapult_invalid_argument);
	}

	// region nemesis transfers

	TEST(TEST_CLASS, CanObserveTransferFromNemesisAccount) {
		// Arrange:
		TestContext context;

		// Act:
		context.notify(MosaicId(123), Amount(111));

		// Assert:
		const auto& outflows = context.outflows();
		EXPECT_EQ(1u, outflows.size());
		EXPECT_EQ(Amount(111), outflows.at(MosaicId(123)));
	}

	TEST(TEST_CLASS, CanObserveMultipleTransfersOfSameTypeFromNemesisAccount) {
		// Arrange:
		TestContext context;

		// Act:
		context.notify(MosaicId(123), Amount(111));
		context.notify(MosaicId(123), Amount(88));
		context.notify(MosaicId(123), Amount(22));
		context.notify(MosaicId(123), Amount(123));

		// Assert:
		const auto& outflows = context.outflows();
		EXPECT_EQ(1u, outflows.size());
		EXPECT_EQ(Amount(111 + 88 + 22 + 123), outflows.at(MosaicId(123)));
	}

	TEST(TEST_CLASS, CanObserveMultipleTransfersOfDifferentTypesFromNemesisAccount) {
		// Arrange:
		TestContext context;

		// Act:
		context.notify(MosaicId(123), Amount(111));
		context.notify(MosaicId(200), Amount(88));
		context.notify(MosaicId(200), Amount(22));
		context.notify(MosaicId(777), Amount(123));
		context.notify(MosaicId(123), Amount(55));

		// Assert:
		const auto& outflows = context.outflows();
		EXPECT_EQ(3u, outflows.size());
		EXPECT_EQ(Amount(111 + 55), outflows.at(MosaicId(123)));
		EXPECT_EQ(Amount(88 + 22), outflows.at(MosaicId(200)));
		EXPECT_EQ(Amount(123), outflows.at(MosaicId(777)));
	}

	// endregion
}}
