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

#include "catapult/model/NemesisNotificationPublisher.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/core/mocks/MockNotificationPublisher.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NemesisNotificationPublisherTests

	// region ExtractNemesisNotificationPublisherOptions

	TEST(TEST_CLASS, ExtractNemesisNotificationPublisherOptions_CanExtractWhenHarvestNetworkFeesAreDisabled) {
		// Arrange:
		auto config = BlockChainConfiguration::Uninitialized();
		test::FillWithRandomData(config.HarvestNetworkFeeSinkAddress);

		// Act:
		auto options = ExtractNemesisNotificationPublisherOptions(config);

		// Assert:
		EXPECT_EQ(0u, options.SpecialAccountAddresses.size());
	}

	TEST(TEST_CLASS, ExtractNemesisNotificationPublisherOptions_CanExtractWhenHarvestNetworkFeesAreEnabled) {
		// Arrange:
		auto config = BlockChainConfiguration::Uninitialized();
		config.HarvestNetworkPercentage = 15;
		test::FillWithRandomData(config.HarvestNetworkFeeSinkAddress);

		// Act:
		auto options = ExtractNemesisNotificationPublisherOptions(config);

		// Assert:
		EXPECT_EQ(1u, options.SpecialAccountAddresses.size());
		EXPECT_EQ(AddressSet{ config.HarvestNetworkFeeSinkAddress }, options.SpecialAccountAddresses);
	}

	// endregion

	// region CreateNemesisNotificationPublisher

	namespace {
		void RunNemesisNotificationPublisherTest(const AddressSet& specialAccountAddresses) {
			// Arrange:
			NemesisNotificationPublisherOptions options;
			options.SpecialAccountAddresses = specialAccountAddresses;

			auto pMockPublisher = std::make_unique<mocks::MockNotificationPublisher>();
			auto pMockPublisherRaw = pMockPublisher.get();
			auto pPublisher = CreateNemesisNotificationPublisher(std::move(pMockPublisher), options);

			auto pMockSubscriber = std::make_unique<mocks::MockNotificationSubscriber>();

			// Act:
			pPublisher->publish(WeakEntityInfo(), *pMockSubscriber);

			// Assert: underlying publisher was called
			EXPECT_EQ(1u, pMockPublisherRaw->numPublishCalls());

			// - one notification was added for each special account
			EXPECT_EQ(specialAccountAddresses.size(), pMockSubscriber->numNotifications());
			EXPECT_EQ(
					std::vector<NotificationType>(specialAccountAddresses.size(), Core_Register_Account_Address_Notification),
					pMockSubscriber->notificationTypes());

			for (const auto& address : specialAccountAddresses)
				EXPECT_TRUE(pMockSubscriber->contains(address));
		}
	}

	TEST(TEST_CLASS, CreateNemesisNotificationPublisher_CanDecorateWithZeroSpecialAccounts) {
		RunNemesisNotificationPublisherTest({});
	}

	TEST(TEST_CLASS, CreateNemesisNotificationPublisher_CanDecorateWithSingleSpecialAccount) {
		RunNemesisNotificationPublisherTest({ test::GenerateRandomByteArray<Address>() });
	}

	TEST(TEST_CLASS, CreateNemesisNotificationPublisher_CanDecorateWithMultipleSpecialAccounts) {
		RunNemesisNotificationPublisherTest({
			test::GenerateRandomByteArray<Address>(),
			test::GenerateRandomByteArray<Address>(),
			test::GenerateRandomByteArray<Address>()
		});
	}

	// endregion
}}
