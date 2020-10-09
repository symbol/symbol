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

#include "zeromq/src/ZeroMqEntityPublisher.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "zeromq/src/PackedFinalizedBlockHeader.h"
#include "zeromq/src/PublisherUtils.h"
#include "catapult/model/Cosignature.h"
#include "catapult/model/Elements.h"
#include "catapult/model/FinalizationRound.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/model/TransactionStatus.h"
#include "zeromq/tests/test/ZeroMqTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS ZeroMqEntityPublisherTests

	namespace {
		model::TransactionInfo ToTransactionInfo(std::unique_ptr<mocks::MockTransaction>&& pTransaction) {
			model::TransactionInfo transactionInfo(std::move(pTransaction));
			transactionInfo.EntityHash = test::GenerateRandomByteArray<Hash256>();
			transactionInfo.MerkleComponentHash = test::GenerateRandomByteArray<Hash256>();
			return transactionInfo;
		}

		model::TransactionElement ToTransactionElement(const mocks::MockTransaction& transaction) {
			model::TransactionElement transactionElement(transaction);
			transactionElement.EntityHash = test::GenerateRandomByteArray<Hash256>();
			transactionElement.MerkleComponentHash = test::GenerateRandomByteArray<Hash256>();
			return transactionElement;
		}

		class EntityPublisherContext : public test::MqContext {
		public:
			using test::MqContext::MqContext;

		public:
			void publishBlockHeader(const model::BlockElement& blockElement) {
				publisher().publishBlockHeader(blockElement);
			}

			void publishDropBlocks(Height height) {
				publisher().publishDropBlocks(height);
			}

			void publishFinalizedBlock(const model::FinalizationRound& round, Height height, const Hash256& hash) {
				publisher().publishFinalizedBlock({ round, height, hash });
			}

			void publishTransaction(TransactionMarker topicMarker, const model::TransactionInfo& transactionInfo, Height height) {
				publisher().publishTransaction(topicMarker, transactionInfo, height);
			}

			void publishTransaction(TransactionMarker topicMarker, const model::TransactionElement& transactionElement, Height height) {
				publisher().publishTransaction(topicMarker, transactionElement, height);
			}

			void publishTransactionHash(TransactionMarker topicMarker, const model::TransactionInfo& transactionInfo) {
				publisher().publishTransactionHash(topicMarker, transactionInfo);
			}

			void publishTransactionStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status) {
				publisher().publishTransactionStatus(transaction, hash, status);
			}

			void publishCosignature(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature& cosignature) {
				publisher().publishCosignature(parentTransactionInfo, cosignature);
			}
		};

		std::shared_ptr<model::UnresolvedAddressSet> GenerateRandomExtractedAddresses() {
			return test::GenerateRandomUnresolvedAddressSetPointer(3);
		}
	}

	// region basic tests

	TEST(TEST_CLASS, CanDestroyPublisherWithUnprocessedMessagesWithoutCrash) {
		// Arrange:
		EntityPublisherContext context;
		context.subscribe(BlockMarker::Drop_Blocks_Marker);

		// Act + Assert:
		context.publishDropBlocks(Height(123));
		context.destroyPublisher();
	}

	TEST(TEST_CLASS, CanUseCustomListenInterface) {
		// Arrange:
		EntityPublisherContext context("127.0.0.1");
		context.subscribe(BlockMarker::Drop_Blocks_Marker);

		// Act:
		context.publishDropBlocks(Height(123));

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		test::AssertDropBlocksMessage(message, Height(123));
	}

	// endregion

	// region publishBlockHeader

	TEST(TEST_CLASS, CanPublishBlockHeader) {
		// Arrange:
		EntityPublisherContext context;
		context.subscribe(BlockMarker::Block_Marker);

		auto pBlock = test::GenerateEmptyRandomBlock();
		auto blockElement = test::BlockToBlockElement(*pBlock);

		// Act:
		context.publishBlockHeader(blockElement);

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		test::AssertBlockHeaderMessage(message, blockElement);
	}

	// endregion

	// region publishDropBlocks

	TEST(TEST_CLASS, CanPublishDropBlocks) {
		// Arrange:
		EntityPublisherContext context;
		context.subscribe(BlockMarker::Drop_Blocks_Marker);

		// Act:
		context.publishDropBlocks(Height(123));

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		test::AssertDropBlocksMessage(message, Height(123));
	}

	// endregion

	// region publishFinalizedBlock

	TEST(TEST_CLASS, CanPublishFinalizedBlock) {
		// Arrange:
		EntityPublisherContext context;
		context.subscribe(BlockMarker::Finalized_Block_Marker);

		auto hash = test::GenerateRandomByteArray<Hash256>();

		// Act:
		context.publishFinalizedBlock({ FinalizationEpoch(24), FinalizationPoint(55) }, Height(123), hash);

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		test::AssertFinalizedBlockMessage(message, { FinalizationEpoch(24), FinalizationPoint(55) }, Height(123), hash);
	}

	// endregion

	// region publishTransaction

	namespace {
		constexpr TransactionMarker Marker = TransactionMarker(12);

		template<typename TAddressesGenerator>
		void AssertCanPublishTransactionInfo(TAddressesGenerator generateAddresses) {
			// Arrange:
			EntityPublisherContext context;
			auto transactionInfo = ToTransactionInfo(mocks::CreateMockTransaction(0));
			Height height(123);
			auto addresses = generateAddresses(transactionInfo);
			context.subscribeAll(Marker, addresses);

			// Act:
			context.publishTransaction(Marker, transactionInfo, height);

			// Assert:
			auto& zmqSocket = context.zmqSocket();
			test::AssertMessages(zmqSocket, Marker, addresses, [&transactionInfo, height](const auto& message, const auto& topic) {
				test::AssertTransactionInfoMessage(message, topic, transactionInfo, height);
			});
		}

		template<typename TAddressesGenerator>
		void AssertCanPublishTransactionElement(TAddressesGenerator generateAddresses) {
			// Arrange:
			EntityPublisherContext context;
			auto pTransaction = mocks::CreateMockTransaction(0);
			auto transactionElement = ToTransactionElement(*pTransaction);
			Height height(123);
			auto addresses = generateAddresses(transactionElement);
			context.subscribeAll(Marker, addresses);

			// Act:
			context.publishTransaction(Marker, transactionElement, height);

			// Assert:
			auto& zmqSocket = context.zmqSocket();
			test::AssertMessages(zmqSocket, Marker, addresses, [&transactionElement, height](const auto& message, const auto& topic) {
				test::AssertTransactionElementMessage(message, topic, transactionElement, height);
			});
		}
	}

	TEST(TEST_CLASS, CanPublishTransaction_TransactionInfo) {
		AssertCanPublishTransactionInfo([](const auto& transactionInfo) {
			return test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
		});
	}

	TEST(TEST_CLASS, CanPublishTransactionToCustomAddresses_TransactionInfo) {
		AssertCanPublishTransactionInfo([](auto& transactionInfo) {
			transactionInfo.OptionalExtractedAddresses = GenerateRandomExtractedAddresses();
			return *transactionInfo.OptionalExtractedAddresses;
		});
	}

	TEST(TEST_CLASS, CanPublishTransaction_TransactionElement) {
		AssertCanPublishTransactionElement([](const auto& transactionElement) {
			return test::ExtractAddresses(test::ToMockTransaction(transactionElement.Transaction));
		});
	}

	TEST(TEST_CLASS, CanPublishTransactionToCustomAddresses_TransactionElement) {
		AssertCanPublishTransactionElement([](auto& transactionElement) {
			transactionElement.OptionalExtractedAddresses = GenerateRandomExtractedAddresses();
			return *transactionElement.OptionalExtractedAddresses;
		});
	}

	TEST(TEST_CLASS, PublishTransactionDeliversMessagesOnlyToRegisteredSubscribers) {
		// Arrange:
		EntityPublisherContext context;
		auto pTransaction = mocks::CreateMockTransaction(0);
		auto recipientAddress = mocks::GetRecipientAddress(*pTransaction);
		auto unresolvedRecipientAddress = extensions::CopyToUnresolvedAddress(recipientAddress);
		auto transactionInfo = ToTransactionInfo(std::move(pTransaction));
		Height height(123);

		// - only subscribe to the recipient address (and not to other addresses like the sender)
		context.subscribeAll(Marker, { unresolvedRecipientAddress });

		// Act:
		context.publishTransaction(Marker, transactionInfo, height);

		// Assert:
		zmq::multipart_t message;
		test::ZmqReceive(message, context.zmqSocket());

		// - only a single message is sent to the recipient address (because that is the only subscribed address)
		auto topic = CreateTopic(Marker, unresolvedRecipientAddress);
		test::AssertTransactionInfoMessage(message, topic, transactionInfo, height);

		// - no other message is pending (e.g. to sender)
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	TEST(TEST_CLASS, PublishTransactionDeliversNoMessagesWhenNoAddressesAreAssociatedWithTransaction) {
		// Arrange:
		EntityPublisherContext context;
		auto transactionInfo = ToTransactionInfo(mocks::CreateMockTransaction(0));
		Height height(123);
		auto addresses = test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
		context.subscribeAll(Marker, addresses);

		// - associate no addresses with the transaction
		transactionInfo.OptionalExtractedAddresses = std::make_shared<model::UnresolvedAddressSet>();

		// Act:
		context.publishTransaction(Marker, transactionInfo, height);

		// Assert: no messages are pending
		test::AssertNoPendingMessages(context.zmqSocket());
	}

	// endregion

	// region publishTransactionHash

	namespace {
		template<typename TAddressesGenerator>
		void AssertCanPublishTransactionHash(TAddressesGenerator generateAddresses) {
			// Arrange:
			EntityPublisherContext context;
			auto pTransaction = mocks::CreateMockTransaction(0);
			auto transactionInfo = ToTransactionInfo(mocks::CreateMockTransaction(0));
			auto addresses = generateAddresses(transactionInfo);
			context.subscribeAll(Marker, addresses);

			// Act:
			context.publishTransactionHash(Marker, transactionInfo);

			// Assert:
			const auto& hash = transactionInfo.EntityHash;
			test::AssertMessages(context.zmqSocket(), Marker, addresses, [&hash](const auto& message, const auto& topic) {
				test::AssertTransactionHashMessage(message, topic, hash);
			});
		}
	}

	TEST(TEST_CLASS, CanPublishTransactionHash) {
		AssertCanPublishTransactionHash([](const auto& transactionInfo) {
			return test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
		});
	}

	TEST(TEST_CLASS, CanPublishTransactionHashToCustomAddresses) {
		AssertCanPublishTransactionHash([](auto& transactionInfo) {
			transactionInfo.OptionalExtractedAddresses = GenerateRandomExtractedAddresses();
			return *transactionInfo.OptionalExtractedAddresses;
		});
	}

	// endregion

	// region publishTransactionStatus

	TEST(TEST_CLASS, CanPublishTransactionStatus) {
		// Arrange:
		EntityPublisherContext context;
		auto pTransaction = mocks::CreateMockTransaction(0);
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto addresses = test::ExtractAddresses(*pTransaction);
		TransactionMarker marker = TransactionMarker::Transaction_Status_Marker;
		context.subscribeAll(marker, addresses);

		// Act:
		context.publishTransactionStatus(*pTransaction, hash, 123);

		// Assert:
		model::TransactionStatus expectedTransactionStatus(hash, pTransaction->Deadline, 123);
		test::AssertMessages(context.zmqSocket(), marker, addresses, [&expectedTransactionStatus](const auto& message, const auto& topic) {
			test::AssertTransactionStatusMessage(message, topic, expectedTransactionStatus);
		});
	}

	// endregion

	// region publishCosignature

	TEST(TEST_CLASS, CanPublishCosignature) {
		// Arrange:
		EntityPublisherContext context;
		auto transactionInfo = ToTransactionInfo(mocks::CreateMockTransaction(0));
		auto cosignature = test::CreateRandomDetachedCosignature();
		auto addresses = test::ExtractAddresses(test::ToMockTransaction(*transactionInfo.pEntity));
		TransactionMarker marker = TransactionMarker::Cosignature_Marker;
		context.subscribeAll(marker, addresses);

		// Act:
		context.publishCosignature(transactionInfo, cosignature);

		// Assert:
		model::DetachedCosignature expectedDetachedCosignature(cosignature, transactionInfo.EntityHash);
		auto& zmqSocket = context.zmqSocket();
		test::AssertMessages(zmqSocket, marker, addresses, [&expectedDetachedCosignature](const auto& message, const auto& topic) {
			test::AssertDetachedCosignatureMessage(message, topic, expectedDetachedCosignature);
		});
	}

	// endregion
}}
