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
#include "catapult/model/NotificationPublisher.h"
#include "catapult/functions.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4267) /* "conversion from 'size_t' to 'uint32_t', possible loss of data" */
#endif
#include <zmq_addon.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace catapult {
	namespace model {
		struct BlockElement;
		struct Cosignature;
		struct FinalizationRound;
		struct Transaction;
		struct TransactionElement;
		struct TransactionInfo;
		class TransactionRegistry;
		struct TransactionStatus;
	}
	namespace zeromq { struct PackedFinalizedBlockHeader; }
}

namespace catapult { namespace zeromq {

	/// Markers for publishing block related messages.
	enum class BlockMarker : uint64_t {
		/// Block.
		Block_Marker = 0x9FF2D8E480CA6A49,

		/// Dropped block(s).
		Drop_Blocks_Marker = 0x5C20D68AEE25B0B0,

		/// Finalized block.
		Finalized_Block_Marker = 0x4D4832A031CE7954
	};

	/// Markers for publishing transaction related messages.
	enum class TransactionMarker : uint8_t {
		/// Confirmed transaction.
		Transaction_Marker = 0x61, // 'a'

		/// Added unconfirmed transaction.
		Unconfirmed_Transaction_Add_Marker = 0x75, // 'u'

		/// Removed unconfirmed transaction.
		Unconfirmed_Transaction_Remove_Marker = 0x72, // 'r'

		/// Transaction status.
		Transaction_Status_Marker = 0x73, // 's'

		/// Added partial transaction.
		Partial_Transaction_Add_Marker = 0x70, // 'p'

		/// Removed partial transaction.
		Partial_Transaction_Remove_Marker = 0x71, // 'q'

		/// Detached cosignature.
		Cosignature_Marker = 0x63 // 'c'
	};

	/// Zeromq entity publisher.
	class ZeroMqEntityPublisher {
	public:
		/// Creates a zeromq entity publisher around \a listenInterface, \a port and \a pNotificationPublisher.
		ZeroMqEntityPublisher(
				const std::string& listenInterface,
				unsigned short port,
				std::unique_ptr<const model::NotificationPublisher>&& pNotificationPublisher);

		~ZeroMqEntityPublisher();

	public:
		/// Publishes the block header in \a blockElement.
		void publishBlockHeader(const model::BlockElement& blockElement);

		/// Publishes the \a height after which all blocks were dropped.
		void publishDropBlocks(Height height);

		/// Publishes a finalized block \a header.
		void publishFinalizedBlock(const PackedFinalizedBlockHeader& header);

		/// Publishes a transaction using \a topicMarker, \a transactionElement and \a height.
		void publishTransaction(TransactionMarker topicMarker, const model::TransactionElement& transactionElement, Height height);

		/// Publishes a transaction using \a topicMarker, \a transactionInfo and \a height.
		void publishTransaction(TransactionMarker topicMarker, const model::TransactionInfo& transactionInfo, Height height);

		/// Publishes a transaction hash using \a topicMarker and \a transactionInfo.
		void publishTransactionHash(TransactionMarker topicMarker, const model::TransactionInfo& transactionInfo);

		/// Publishes a transaction status composed of \a transaction, \a hash and \a status.
		void publishTransactionStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status);

		/// Publishes \a cosignature associated with parent transaction info (\a parentTransactionInfo).
		void publishCosignature(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature& cosignature);

	private:
		struct WeakTransactionInfo;
		using MessagePayloadBuilder = consumer<zmq::multipart_t&>;

		void publishTransaction(TransactionMarker topicMarker, const WeakTransactionInfo& transactionInfo, Height height);
		void publish(
				const std::string& topicName,
				TransactionMarker topicMarker,
				const WeakTransactionInfo& transactionInfo,
				const MessagePayloadBuilder& payloadBuilder);

	private:
		class SynchronizedPublisher;
		std::unique_ptr<const model::NotificationPublisher> m_pNotificationPublisher;
		std::unique_ptr<SynchronizedPublisher> m_pSynchronizedPublisher;
	};
}}
