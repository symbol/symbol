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

#include "ZeroMqTestUtils.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "zeromq/src/PackedFinalizedBlockHeader.h"
#include "zeromq/src/PublisherUtils.h"
#include "catapult/model/Address.h"
#include "catapult/model/Cosignature.h"
#include "catapult/model/Elements.h"
#include "catapult/model/FinalizationRound.h"
#include "catapult/model/Transaction.h"
#include "catapult/model/TransactionStatus.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;

		template<typename TEntity>
		void AssertMessagePart(const zmq::message_t& messagePart, const TEntity* pExpectedData, size_t expectedSize) {
			ASSERT_EQ(expectedSize, messagePart.size());

			const auto* pExpected = test::AsVoidPointer(pExpectedData);
			EXPECT_EQ_MEMORY(pExpected, messagePart.data(), messagePart.size());
		}
	}

	const mocks::MockTransaction& ToMockTransaction(const model::Transaction& transaction) {
		return static_cast<const mocks::MockTransaction&>(transaction);
	}

	model::UnresolvedAddressSet ToAddresses(const std::vector<Key>& keys) {
		model::UnresolvedAddressSet addresses;
		for (const auto& key : keys)
			addresses.insert(extensions::CopyToUnresolvedAddress(model::PublicKeyToAddress(key, Network_Identifier)));

		return addresses;
	}

	model::UnresolvedAddressSet ExtractAddresses(const mocks::MockTransaction& transaction) {
		auto networkIdentifier = transaction.Network;
		auto signerAddress = extensions::CopyToUnresolvedAddress(model::PublicKeyToAddress(
				transaction.SignerPublicKey,
				networkIdentifier));
		auto recipientAddress = extensions::CopyToUnresolvedAddress(model::PublicKeyToAddress(
				transaction.RecipientPublicKey,
				networkIdentifier));
		return { signerAddress, recipientAddress };
	}

	model::TransactionInfo RemoveExtractedAddresses(model::TransactionInfo&& transactionInfo) {
		transactionInfo.OptionalExtractedAddresses = nullptr;
		return std::move(transactionInfo);
	}

	std::vector<model::TransactionInfo> RemoveExtractedAddresses(std::vector<model::TransactionInfo>&& transactionInfos) {
		for (auto& transactionInfo : transactionInfos)
			transactionInfo.OptionalExtractedAddresses = nullptr;

		return std::move(transactionInfos);
	}

	void SubscribeForAddresses(zmq::socket_t& socket, zeromq::TransactionMarker marker, const model::UnresolvedAddressSet& addresses) {
		for (const auto& address : addresses) {
			auto topic = zeromq::CreateTopic(marker, address);
			auto topicBuffer = zmq::const_buffer(reinterpret_cast<const void*>(topic.data()), topic.size());
			socket.set(zmq::sockopt::subscribe, topicBuffer);
		}
	}

	void ZmqReceive(zmq::multipart_t& message, zmq::socket_t& socket) {
		constexpr uint8_t Max_Attempts = 20;
		auto counter = 0u;
		auto receiveSuccess = false;
		while (Max_Attempts > counter && !receiveSuccess) {
			receiveSuccess = message.recv(socket);
			++counter;
		}

		if (Max_Attempts <= counter)
			CATAPULT_THROW_RUNTIME_ERROR("ZmqReceive failed to read from zmq socket");
	}

	void AssertBlockHeaderMessage(const zmq::multipart_t& message, const model::BlockElement& blockElement) {
		ASSERT_EQ(4u, message.size());

		auto marker = zeromq::BlockMarker::Block_Marker;
		AssertMessagePart(message[0], &marker, sizeof(zeromq::BlockMarker));
		AssertMessagePart(message[1], &blockElement.Block, sizeof(model::BlockHeader));
		AssertMessagePart(message[2], &blockElement.EntityHash, Hash256::Size);
		AssertMessagePart(message[3], &blockElement.GenerationHash, Hash256::Size);
	}

	void AssertDropBlocksMessage(const zmq::multipart_t& message, Height height) {
		ASSERT_EQ(2u, message.size());

		auto marker = zeromq::BlockMarker::Drop_Blocks_Marker;
		AssertMessagePart(message[0], &marker, sizeof(zeromq::BlockMarker));
		AssertMessagePart(message[1], &height, sizeof(Height));
	}

	void AssertFinalizedBlockMessage(
			const zmq::multipart_t& message,
			const model::FinalizationRound& round,
			Height height,
			const Hash256& hash) {
		ASSERT_EQ(2u, message.size());

		auto marker = zeromq::BlockMarker::Finalized_Block_Marker;
		auto expectedHeader = zeromq::PackedFinalizedBlockHeader{ round, height, hash };
		AssertMessagePart(message[0], &marker, sizeof(zeromq::BlockMarker));
		AssertMessagePart(message[1], &expectedHeader, sizeof(zeromq::PackedFinalizedBlockHeader));
	}

	void AssertTransactionElementMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::TransactionElement& transactionElement,
			Height height) {
		ASSERT_EQ(5u, message.size());

		const auto& transaction = transactionElement.Transaction;
		AssertMessagePart(message[0], topic.data(), topic.size());
		AssertMessagePart(message[1], &transaction, transaction.Size);
		AssertMessagePart(message[2], &transactionElement.EntityHash, Hash256::Size);
		AssertMessagePart(message[3], &transactionElement.MerkleComponentHash, Hash256::Size);
		AssertMessagePart(message[4], &height, sizeof(Height));
	}

	void AssertTransactionInfoMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::TransactionInfo& transactionInfo,
			Height height) {
		ASSERT_EQ(5u, message.size());

		const auto& transaction = *transactionInfo.pEntity;
		AssertMessagePart(message[0], topic.data(), topic.size());
		AssertMessagePart(message[1], &transaction, transaction.Size);
		AssertMessagePart(message[2], &transactionInfo.EntityHash, Hash256::Size);
		AssertMessagePart(message[3], &transactionInfo.MerkleComponentHash, Hash256::Size);
		AssertMessagePart(message[4], &height, sizeof(Height));
	}

	void AssertTransactionHashMessage(const zmq::multipart_t& message, const std::vector<uint8_t>& topic, const Hash256& hash) {
		ASSERT_EQ(2u, message.size());

		AssertMessagePart(message[0], topic.data(), topic.size());
		AssertMessagePart(message[1], &hash, Hash256::Size);
	}

	void AssertTransactionStatusMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::TransactionStatus& transactionStatus) {
		ASSERT_EQ(2u, message.size());

		AssertMessagePart(message[0], topic.data(), topic.size());
		AssertMessagePart(message[1], &transactionStatus, sizeof(model::TransactionStatus));
	}

	void AssertDetachedCosignatureMessage(
			const zmq::multipart_t& message,
			const std::vector<uint8_t>& topic,
			const model::DetachedCosignature& detachedCosignature) {
		ASSERT_EQ(2u, message.size());

		AssertMessagePart(message[0], topic.data(), topic.size());
		AssertMessagePart(message[1], &detachedCosignature, sizeof(model::DetachedCosignature));
	}

	void AssertMessages(
			zmq::socket_t& zmqSocket,
			zeromq::TransactionMarker marker,
			const model::UnresolvedAddressSet& addresses,
			const AssertMessage& assertMessage) {
		zmq::multipart_t message;

		// Assert: each address should get a message
		auto addressesCopy = addresses;
		for (auto i = 0u; i < addresses.size(); ++i) {
			ZmqReceive(message, zmqSocket);

			const auto* pAddressData = reinterpret_cast<const uint8_t*>(message[0].data()) + 1;
			const auto& address = reinterpret_cast<const UnresolvedAddress&>(*pAddressData);
			EXPECT_EQ(1u, addressesCopy.erase(address)) << "address " << address;

			auto topic = CreateTopic(marker, address);
			assertMessage(message, topic);
		}

		// - all addresses were used exactly once
		EXPECT_TRUE(addressesCopy.empty());
	}

	void AssertNoPendingMessages(zmq::socket_t& zmqSocket) {
		zmq::multipart_t message;
		ASSERT_FALSE(message.recv(zmqSocket));
	}
}}
