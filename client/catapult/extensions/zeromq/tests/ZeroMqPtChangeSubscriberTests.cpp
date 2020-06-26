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

#include "zeromq/src/ZeroMqPtChangeSubscriber.h"
#include "zeromq/src/PublisherUtils.h"
#include "catapult/model/Cosignature.h"
#include "zeromq/tests/test/ZeroMqTestUtils.h"
#include "zeromq/tests/test/ZeroMqTransactionsChangeTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS ZeroMqPtChangeSubscriberTests

	namespace {
		class MqSubscriberContext : public test::MqContextT<cache::PtChangeSubscriber> {
		public:
			using MqContext::subscribeAll;

		public:
			MqSubscriberContext() : MqContextT(CreateZeroMqPtChangeSubscriber)
			{}

		public:
			void notifyAddPartial(const model::TransactionInfo& transactionInfo) {
				cache::PtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				notifyAddPartials(transactionInfos);
			}

			void notifyAddPartials(const cache::PtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber().notifyAddPartials(transactionInfos);
			}

			void notifyAddCosignature(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature& cosignature) {
				subscriber().notifyAddCosignature(parentTransactionInfo, cosignature);
			}

			void notifyRemovePartial(const model::TransactionInfo& transactionInfo) {
				cache::PtChangeSubscriber::TransactionInfos transactionInfos;
				transactionInfos.emplace(transactionInfo.copy());
				notifyRemovePartials(transactionInfos);
			}

			void notifyRemovePartials(const cache::PtChangeSubscriber::TransactionInfos& transactionInfos) {
				subscriber().notifyRemovePartials(transactionInfos);
			}

			void flush() {
				subscriber().flush();
			}

		public:
			template<typename TTransactionInfos>
			void subscribeAll(TransactionMarker topicMarker, const TTransactionInfos& transactionInfos) {
				for (const auto& transactionInfo : transactionInfos) {
					auto addresses = test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
					test::SubscribeForAddresses(zmqSocket(), topicMarker, addresses);
				}

				waitForReceiveSuccess();
			}
		};
	}

	// region basic tests

	TEST(TEST_CLASS, SubscriberDoesNotReceiveDataOnDifferentTopic) {
		// Arrange:
		uint64_t topic(0x12345678);
		MqSubscriberContext context;
		context.subscribe(topic);

		auto transactionInfo = test::CreateRandomTransactionInfo();

		// Act:
		context.notifyAddPartial(transactionInfo);

		// Assert:
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	namespace {
		constexpr size_t Num_Transactions = 5;
		constexpr auto Add_Marker = TransactionMarker::Partial_Transaction_Add_Marker;
		constexpr auto Remove_Marker = TransactionMarker::Partial_Transaction_Remove_Marker;
	}

	// region notifyAddPartial

	namespace {
		template<typename TAddAll>
		void AssertCanAddMultipleTransactions(TAddAll addAll) {
			test::AssertCanAddMultipleTransactions<MqSubscriberContext>(Add_Marker, Num_Transactions, addAll);
		}
	}

	TEST(TEST_CLASS, CanAddSinglePartialTransaction) {
		test::AssertCanAddSingleTransaction<MqSubscriberContext>(Add_Marker, [](auto& context, const auto& transactionInfo) {
			context.notifyAddPartial(transactionInfo);
		});
	}

	TEST(TEST_CLASS, CanAddMultiplePartialTransactions_SingleCall) {
		AssertCanAddMultipleTransactions([](auto& context, const auto& transactionInfos) {
			context.notifyAddPartials(transactionInfos);
		});
	}

	TEST(TEST_CLASS, CanAddMultiplePartialTransactions_MultipleCalls) {
		AssertCanAddMultipleTransactions([](auto& context, const auto& transactionInfos) {
			for (const auto& transactionInfo : transactionInfos)
				context.notifyAddPartial(transactionInfo);
		});
	}

	// endregion

	// region notifyAddCosignature

	TEST(TEST_CLASS, CanAddSingleCosignature) {
		// Arrange:
		MqSubscriberContext context;
		auto marker = TransactionMarker::Cosignature_Marker;
		auto transactionInfo = test::RemoveExtractedAddresses(test::CreateRandomTransactionInfo());
		auto cosignature = test::CreateRandomDetachedCosignature();
		auto addresses = test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
		context.subscribeAll(marker, addresses);

		// Act:
		context.notifyAddCosignature(transactionInfo, cosignature);

		// Assert:
		model::DetachedCosignature detachedCosignature(cosignature, transactionInfo.EntityHash);
		test::AssertMessages(context.zmqSocket(), marker, addresses, [&detachedCosignature](const auto& message, const auto& topic) {
			test::AssertDetachedCosignatureMessage(message, topic, detachedCosignature);
		});

		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	// region notifyRemovePartial

	namespace {
		template<typename TRemoveAll>
		void AssertCanRemoveMultipleTransactions(TRemoveAll removeAll) {
			test::AssertCanRemoveMultipleTransactions<MqSubscriberContext>(Remove_Marker, Num_Transactions, removeAll);
		}
	}

	TEST(TEST_CLASS, CanRemoveSinglePartialTransaction) {
		test::AssertCanRemoveSingleTransaction<MqSubscriberContext>(Remove_Marker, [](auto& context, const auto& transactionInfo) {
			context.notifyRemovePartial(transactionInfo);
		});
	}

	TEST(TEST_CLASS, CanRemoveMultiplePartialTransactions_SingleCall) {
		AssertCanRemoveMultipleTransactions([](auto& context, const auto& transactionInfos) {
			context.notifyRemovePartials(transactionInfos);
		});
	}

	TEST(TEST_CLASS, CanRemoveMultiplePartialTransactions_MultipleCalls) {
		AssertCanRemoveMultipleTransactions([](auto& context, const auto& transactionInfos) {
			for (const auto& transactionInfo : transactionInfos)
				context.notifyRemovePartial(transactionInfo);
		});
	}

	// endregion

	// region flush

	TEST(TEST_CLASS, FlushDoesNotSendMessages) {
		test::AssertFlushDoesNotSendMessages<MqSubscriberContext>();
	}

	// endregion
}}
