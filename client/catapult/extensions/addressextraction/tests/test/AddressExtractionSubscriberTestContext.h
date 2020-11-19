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

#pragma once
#include "addressextraction/src/AddressExtractor.h"
#include "catapult/model/Elements.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/core/mocks/MockNotificationPublisher.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Test context for address extraction subscriber tests.
	template<typename TSubscriber>
	class AddressExtractionSubscriberTestContext {
	private:
		using SubscriberFactory = std::function<std::unique_ptr<TSubscriber> (const addressextraction::AddressExtractor&)>;

	public:
		/// Creates a test context around \a subscriberFactory.
		explicit AddressExtractionSubscriberTestContext(const SubscriberFactory& subscriberFactory)
				: m_subscriberFactory(subscriberFactory)
				, m_pNotificationPublisher(std::make_unique<mocks::MockNotificationPublisher>())
				, m_notificationPublisher(*m_pNotificationPublisher)
				, m_extractor(std::move(m_pNotificationPublisher))
		{}

	public:
		/// Asserts that \a action results in no extracted addresses.
		template<typename TAction>
		void assertNoExtractions(TAction action) {
			// Arrange: create subscriber
			auto pSubscriber = m_subscriberFactory(m_extractor);

			// Act:
			action(*pSubscriber);

			// Assert:
			EXPECT_EQ(0u, m_notificationPublisher.numPublishCalls());
		}

		/// Asserts that \a action results in extracted addresses.
		template<typename TAction>
		void assertTransactionInfoExtractions(TAction action) {
			// Arrange: create subscriber
			auto pSubscriber = m_subscriberFactory(m_extractor);

			auto transactionInfo = CreateRandomTransactionInfo();
			transactionInfo.OptionalExtractedAddresses = nullptr;

			// Act:
			action(*pSubscriber, transactionInfo);

			// Assert:
			EXPECT_EQ(1u, m_notificationPublisher.numPublishCalls());
			EXPECT_TRUE(!!transactionInfo.OptionalExtractedAddresses);
		}

		/// Asserts that \a action results in extracted addresses.
		template<typename TAction>
		void assertTransactionInfosExtractions(TAction action) {
			// Arrange: create subscriber
			auto pSubscriber = m_subscriberFactory(m_extractor);
			auto pAddresses1 = std::make_shared<model::UnresolvedAddressSet>();
			auto pAddresses3 = std::make_shared<model::UnresolvedAddressSet>();

			// - create two infos with addresses and two without
			auto transactionInfos = CreateTransactionInfos(4);
			transactionInfos[0].OptionalExtractedAddresses = nullptr;
			transactionInfos[1].OptionalExtractedAddresses = pAddresses1;
			transactionInfos[2].OptionalExtractedAddresses = nullptr;
			transactionInfos[3].OptionalExtractedAddresses = pAddresses3;
			auto transactionInfoSet = CopyTransactionInfosToSet(transactionInfos);

			// Act:
			action(*pSubscriber, transactionInfoSet);

			// Assert:
			EXPECT_EQ(2u, m_notificationPublisher.numPublishCalls());

			// - all have addresses
			for (auto& transactionInfo : transactionInfoSet)
				EXPECT_TRUE(!!transactionInfo.OptionalExtractedAddresses);

			// - the previously existing addresses are unchanged
			EXPECT_EQ(pAddresses1, transactionInfoSet.find(transactionInfos[1])->OptionalExtractedAddresses);
			EXPECT_EQ(pAddresses3, transactionInfoSet.find(transactionInfos[3])->OptionalExtractedAddresses);
		}

		/// Asserts that \a action results in extracted addresses.
		template<typename TAction>
		void assertBlockElementExtractions(TAction action) {
			// Arrange: create subscriber
			auto pSubscriber = m_subscriberFactory(m_extractor);
			auto pAddresses1 = std::make_shared<model::UnresolvedAddressSet>();
			auto pAddresses3 = std::make_shared<model::UnresolvedAddressSet>();

			// - create two elements with addresses and two without
			model::Block block;
			model::BlockElement blockElement(block);
			auto transactions = GenerateRandomTransactions(4);
			for (const auto& pTransaction : transactions)
				blockElement.Transactions.push_back(model::TransactionElement(*pTransaction));

			blockElement.Transactions[0].OptionalExtractedAddresses = nullptr;
			blockElement.Transactions[1].OptionalExtractedAddresses = pAddresses1;
			blockElement.Transactions[2].OptionalExtractedAddresses = nullptr;
			blockElement.Transactions[3].OptionalExtractedAddresses = pAddresses3;

			// Act:
			action(*pSubscriber, blockElement);

			// Assert:
			EXPECT_EQ(2u, m_notificationPublisher.numPublishCalls());

			// - all have addresses
			for (auto& transactionElement : blockElement.Transactions)
				EXPECT_TRUE(!!transactionElement.OptionalExtractedAddresses);

			// - the previously existing addresses are unchanged
			EXPECT_EQ(pAddresses1, blockElement.Transactions[1].OptionalExtractedAddresses);
			EXPECT_EQ(pAddresses3, blockElement.Transactions[3].OptionalExtractedAddresses);
		}

	private:
		SubscriberFactory m_subscriberFactory;
		std::unique_ptr<mocks::MockNotificationPublisher> m_pNotificationPublisher; // moved into m_extractor
		mocks::MockNotificationPublisher& m_notificationPublisher;
		addressextraction::AddressExtractor m_extractor;
	};
}}
