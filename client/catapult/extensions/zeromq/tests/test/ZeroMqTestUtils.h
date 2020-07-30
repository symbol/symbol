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

#pragma once
#include "zeromq/src/ZeroMqEntityPublisher.h"
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionRegistry.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/types.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"
#include <unordered_set>
#include <vector>
#include <zmq_addon.hpp>

namespace catapult {
	namespace model {
		struct BlockElement;
		struct DetachedCosignature;
		struct TransactionElement;
		struct TransactionInfo;
		struct TransactionStatus;
	}
	namespace zeromq { class ZeroMqEntityPublisher; }
}

namespace catapult { namespace test {

	using AssertMessage = consumer<const zmq::multipart_t&, const std::vector<uint8_t>&>;

	/// Converts a given \a transaction to a mock transaction.
	const mocks::MockTransaction& ToMockTransaction(const model::Transaction& transaction);

	/// Converts a given vector of \a keys to a set of addresses.
	model::UnresolvedAddressSet ToAddresses(const std::vector<Key>& keys);

	/// Extracts all addresses from a mock \a transaction.
	model::UnresolvedAddressSet ExtractAddresses(const mocks::MockTransaction& transaction);

	/// Removes all extracted addresses from \a transactionInfo.
	model::TransactionInfo RemoveExtractedAddresses(model::TransactionInfo&& transactionInfo);

	/// Removes all extracted addresses from \a transactionInfos.
	std::vector<model::TransactionInfo> RemoveExtractedAddresses(std::vector<model::TransactionInfo>&& transactionInfos);

	/// Subscribes \a socket to topics created from \a marker and \a addresses.
	void SubscribeForAddresses(zmq::socket_t& socket, zeromq::TransactionMarker marker, const model::UnresolvedAddressSet& addresses);

	/// Attempts to receive a single \a message using \a socket.
	void ZmqReceive(zmq::multipart_t& message, zmq::socket_t& socket);

	/// Asserts that the given \a message and \a blockElement have matching block header data.
	void AssertBlockHeaderMessage(const zmq::multipart_t& message, const model::BlockElement& blockElement);

	/// Asserts that the given \a message is equivalent to a drop blocks message with \a height.
	void AssertDropBlocksMessage(const zmq::multipart_t& message, Height height);

	/// Asserts that the given \a message is equivalent to a finalized block message with \a height, \a hash and \a point.
	void AssertFinalizedBlockMessage(const zmq::multipart_t& message, Height height, const Hash256& hash, FinalizationPoint point);

	/// Asserts that the given \a message has \a topic as first part and matches the data in \a transactionElement and \a height.
	void AssertTransactionElementMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::TransactionElement& transactionElement,
			Height height);

	/// Asserts that the given \a message has \a topic as first part and matches the data in \a transactionInfo and \a height.
	void AssertTransactionInfoMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::TransactionInfo& transactionInfo,
			Height height);

	/// Asserts that the given \a message has \a topic as first part and matches the data in \a hash.
	void AssertTransactionHashMessage(const zmq::multipart_t& message, const std::vector<uint8_t>& topic, const Hash256& hash);

	/// Asserts that the given \a message has \a topic as first part and matches the data in \a transactionStatus.
	void AssertTransactionStatusMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::TransactionStatus& transactionStatus);

	/// Asserts that the given \a message has \a topic as first part and matches the data in \a detachedCosignature.
	void AssertDetachedCosignatureMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::DetachedCosignature& detachedCosignature);

	/// Asserts that all pending messages of the socket (\a zmqSocket) that are subscribed to the topic composed of
	/// \a marker and \a address can be asserted using \a assertMessage.
	void AssertMessages(
			zmq::socket_t& zmqSocket,
			zeromq::TransactionMarker marker,
			const model::UnresolvedAddressSet& addresses,
			const AssertMessage& assertMessage);

	/// Asserts that the socket (\a zmqSocket) has no messages pending.
	void AssertNoPendingMessages(zmq::socket_t& zmqSocket);

	/// Base context for all zeromq related contexts.
	class MqContext {
	public:
		/// Creates a message queue context.
		MqContext()
				: m_registry(mocks::CreateDefaultTransactionRegistry())
				, m_pZeroMqEntityPublisher(std::make_shared<zeromq::ZeroMqEntityPublisher>(
						GetDefaultLocalHostZmqPort(),
						model::CreateNotificationPublisher(m_registry, UnresolvedMosaicId())))
				, m_zmqSocket(m_zmqContext, ZMQ_SUB) {
			m_zmqSocket.setsockopt(ZMQ_RCVTIMEO, 10);
			m_zmqSocket.connect("tcp://localhost:" + std::to_string(GetDefaultLocalHostZmqPort()));
		}

	public:
		/// Subscribes to \a topic.
		template<typename TTopic>
		void subscribe(TTopic topic) {
			m_zmqSocket.setsockopt(ZMQ_SUBSCRIBE, &topic, sizeof(TTopic));

			// make an additional subscription and wait until messages can be received
			waitForReceiveSuccess();
		}

		/// Subscribes to all topics using \a marker and \a addresses.
		void subscribeAll(zeromq::TransactionMarker marker, const model::UnresolvedAddressSet& addresses) {
			SubscribeForAddresses(m_zmqSocket, marker, addresses);

			// make an additional subscription and wait until messages can be received
			waitForReceiveSuccess();
		}

		/// Gets the publisher.
		zeromq::ZeroMqEntityPublisher& publisher() {
			return *m_pZeroMqEntityPublisher;
		}

		/// Destroys the underlying publisher.
		void destroyPublisher() {
			m_pZeroMqEntityPublisher.reset();
		}

		/// Gets the zmq socket.
		zmq::socket_t& zmqSocket() {
			return m_zmqSocket;
		}

		void waitForReceiveSuccess() {
			constexpr uint8_t Max_Attempts = 20;
			auto marker = zeromq::BlockMarker::Drop_Blocks_Marker;
			m_zmqSocket.setsockopt(ZMQ_SUBSCRIBE, &marker, sizeof(zeromq::BlockMarker));
			auto receiveSuccess = false;
			auto counter = 0u;
			zmq::multipart_t message;
			while (Max_Attempts > counter && !receiveSuccess) {
				m_pZeroMqEntityPublisher->publishDropBlocks(Height(1357));
				receiveSuccess = message.recv(m_zmqSocket);
				++counter;
			}

			// ensure there are no pending messages
			for (auto i = 0u; i < 10; ++i) {
				receiveSuccess = message.recv(m_zmqSocket);
				if (receiveSuccess)
					CATAPULT_LOG(info) << "received message with " << message.size() << " parts while emptying message queue";

				test::Sleep(5);
			}

			if (Max_Attempts <= counter)
				CATAPULT_LOG(warning) << "waitForReceiveSuccess failed with " << counter << " attempts";
		}

	protected:
		/// Gets the transaction registry.
		const model::TransactionRegistry& registry() const {
			return m_registry;
		}

	private:
		static unsigned short GetDefaultLocalHostZmqPort() {
			return static_cast<unsigned short>(GetLocalHostPort() + 2);
		}

	private:
		model::TransactionRegistry m_registry;
		std::shared_ptr<zeromq::ZeroMqEntityPublisher> m_pZeroMqEntityPublisher;
		zmq::context_t m_zmqContext;
		zmq::socket_t m_zmqSocket;
	};

	/// Message queue context for a subscriber.
	template<typename TSubscriber>
	class MqContextT : public MqContext {
	private:
		using SubscriberCreator = std::function<std::unique_ptr<TSubscriber> (zeromq::ZeroMqEntityPublisher&)>;

	public:
		/// Creates a message queue context using the supplied subscriber creator (\a subscriberCreator).
		explicit MqContextT(const SubscriberCreator& subscriberCreator)
				: m_pNotificationPublisher(model::CreateNotificationPublisher(registry(), UnresolvedMosaicId()))
				, m_pZeroMqSubscriber(subscriberCreator(publisher()))
		{}

	public:
		/// Gets the notification publisher.
		const model::NotificationPublisher& notificationPublisher() const {
			return *m_pNotificationPublisher;
		}

		/// Gets the subscriber.
		TSubscriber& subscriber() const {
			return *m_pZeroMqSubscriber;
		}

	private:
		std::unique_ptr<const model::NotificationPublisher> m_pNotificationPublisher;
		std::shared_ptr<TSubscriber> m_pZeroMqSubscriber;
	};
}}
