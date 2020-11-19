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
#include "ZeroMqTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"

namespace catapult { namespace test {

	// region common

	/// Asserts that \a operation raises a single message with \a marker that is checked by \a assertMessage.
	template<typename TMqSubscriberContext, typename TOperation, typename TAssertMessage>
	void AssertRaiseSingleMessage(zeromq::TransactionMarker marker, TOperation operation, TAssertMessage assertMessage) {
		// Arrange:
		TMqSubscriberContext context;
		auto transactionInfo = RemoveExtractedAddresses(CreateRandomTransactionInfo());
		auto addresses = ExtractAddresses(ToMockTransaction(*transactionInfo.pEntity));
		context.subscribeAll(marker, addresses);

		// Act:
		operation(context, transactionInfo);

		// Assert:
		auto& zmqSocket = context.zmqSocket();
		AssertMessages(zmqSocket, marker, addresses, [assertMessage, &transactionInfo](const auto& message, const auto& topic) {
			assertMessage(message, topic, transactionInfo);
		});

		AssertNoPendingMessages(context.zmqSocket());
	}

	/// Asserts that \a operation raises \a numTransactions messages with \a marker that are checked individually by \a assertMessage.
	template<typename TMqSubscriberContext, typename TOperation, typename TAssertMessage>
	void AssertRaiseMultipleMessages(
			zeromq::TransactionMarker marker,
			size_t numTransactions,
			TOperation operation,
			TAssertMessage assertMessage) {
		// Arrange: insert transactions into a set so that they are processed in deterministic order
		TMqSubscriberContext context;
		auto transactionInfos = CopyTransactionInfosToSet(RemoveExtractedAddresses(CreateTransactionInfos(numTransactions)));
		context.subscribeAll(marker, transactionInfos);

		// Act:
		operation(context, transactionInfos);

		// Assert:
		auto& zmqSocket = context.zmqSocket();
		for (const auto& transactionInfo : transactionInfos) {
			auto addresses = ExtractAddresses(ToMockTransaction(*transactionInfo.pEntity));
			AssertMessages(zmqSocket, marker, addresses, [assertMessage, &transactionInfo](const auto& message, const auto& topic) {
				assertMessage(message, topic, transactionInfo);
			});
		}

		AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	// region add

	/// Asserts that a single transaction can be added by \a add and sends a message with \a marker.
	template<typename TMqSubscriberContext, typename TAdd>
	void AssertCanAddSingleTransaction(zeromq::TransactionMarker marker, TAdd add) {
		// Assert:
		AssertRaiseSingleMessage<TMqSubscriberContext>(
				marker,
				[add](auto& context, const auto& transactionInfo) {
					add(context, transactionInfo);
				},
				[](const auto& message, const auto& topic, const auto& transactionInfo) {
					AssertTransactionInfoMessage(message, topic, transactionInfo, Height());
				});
	}

	/// Asserts that \a numTransactions transactions can be added by \a addAll and sends messages with \a marker.
	template<typename TMqSubscriberContext, typename TAddAll>
	void AssertCanAddMultipleTransactions(zeromq::TransactionMarker marker, size_t numTransactions, TAddAll addAll) {
		// Assert:
		AssertRaiseMultipleMessages<TMqSubscriberContext>(
				marker,
				numTransactions,
				[addAll](auto& context, const auto& transactionInfos) {
					addAll(context, transactionInfos);
				},
				[](const auto& message, const auto& topic, const auto& transactionInfo) {
					AssertTransactionInfoMessage(message, topic, transactionInfo, Height());
				});
	}

	// endregion

	// region remove

	/// Asserts that a single transaction can be removed by \a remove and sends a message with \a marker.
	template<typename TMqSubscriberContext, typename TRemove>
	void AssertCanRemoveSingleTransaction(zeromq::TransactionMarker marker, TRemove remove) {
		// Assert:
		AssertRaiseSingleMessage<TMqSubscriberContext>(
				marker,
				[remove](auto& context, const auto& transactionInfo) {
					remove(context, transactionInfo);
				},
				[](const auto& message, const auto& topic, const auto& transactionInfo) {
					AssertTransactionHashMessage(message, topic, transactionInfo.EntityHash);
				});
	}

	/// Asserts that \a numTransactions transactions can be removed by \a removeAll and sends messages with \a marker.
	template<typename TMqSubscriberContext, typename TRemoveAll>
	void AssertCanRemoveMultipleTransactions(zeromq::TransactionMarker marker, size_t numTransactions, TRemoveAll removeAll) {
		// Assert:
		AssertRaiseMultipleMessages<TMqSubscriberContext>(
				marker,
				numTransactions,
				[removeAll](auto& context, const auto& transactionInfos) {
					removeAll(context, transactionInfos);
				},
				[](const auto& message, const auto& topic, const auto& transactionInfo) {
					AssertTransactionHashMessage(message, topic, transactionInfo.EntityHash);
				});
	}

	// endregion

	// region flush

	/// Asserts that flush has no effect.
	template<typename TContext>
	void AssertFlushDoesNotSendMessages() {
		// Arrange: subscribe to everything
		TContext context;
		context.subscribe("");

		// Act:
		context.flush();

		// Assert:
		AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion
}}
