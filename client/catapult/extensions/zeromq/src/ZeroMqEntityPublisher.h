#pragma once
#include "catapult/model/NotificationPublisher.h"
#include "catapult/functions.h"
#include <zmq_addon.hpp>

namespace catapult {
	namespace model {
		struct BlockElement;
		struct Transaction;
		struct TransactionElement;
		struct TransactionInfo;
		class TransactionRegistry;
		struct TransactionStatus;
	}
}

namespace catapult { namespace zeromq {

	/// Markers for publishing block related messages.
	enum class BlockMarker : uint64_t {
		/// A block.
		Block_Marker = 0x9FF2D8E480CA6A49,

		/// A dropped block.
		Drop_Blocks_Marker = 0x5C20D68AEE25B0B0
	};

	/// Markers for publishing transaction related messages.
	enum class TransactionMarker : uint8_t {
		/// A confirmed transaction.
		Transaction_Marker = 0x61, // 'a'

		/// An added unconfirmed transaction.
		Unconfirmed_Transaction_Add_Marker = 0x75, // 'u'

		/// A removed unconfirmed transaction.
		Unconfirmed_Transaction_Remove_Marker = 0x72, // 'r'

		/// A transaction status.
		Transaction_Status_Marker = 0x73, // 's'

		/// An added partial transaction.
		Partial_Transaction_Add_Marker = 0x70, // 'p'

		/// A removed partial transaction.
		Partial_Transaction_Remove_Marker = 0x71, // 'q'

		/// A detached cosignature.
		Cosignature_Marker = 0x63 // 'c'
	};

	/// A zeromq entity publisher.
	class ZeroMqEntityPublisher {
	public:
		/// Creates a zeromq entity publisher around \a port and \a pNotificationPublisher.
		explicit ZeroMqEntityPublisher(unsigned short port, std::unique_ptr<model::NotificationPublisher>&& pNotificationPublisher);

		~ZeroMqEntityPublisher();

	public:
		/// Publishes the block header in \a blockElement.
		void publishBlockHeader(const model::BlockElement& blockElement);

		/// Publishes the \a height after which all blocks were dropped.
		void publishDropBlocks(Height height);

		/// Publishes a transaction using \a topicMarker, \a transactionElement and \a height.
		void publishTransaction(TransactionMarker topicMarker, const model::TransactionElement& transactionElement, Height height);

		/// Publishes a transaction using \a topicMarker, \a transactionInfo and \a height.
		void publishTransaction(TransactionMarker topicMarker, const model::TransactionInfo& transactionInfo, Height height);

		/// Publishes a transaction hash using \a topicMarker and \a transactionInfo.
		void publishTransactionHash(TransactionMarker topicMarker, const model::TransactionInfo& transactionInfo);

		/// Publishes a transaction status composed of \a transaction, \a hash and \a status.
		void publishTransactionStatus(const model::Transaction& transaction, const Hash256& hash, uint32_t status);

		/// Publishes a cosignature composed of transaction info (\a parentTransactionInfo), \a signer and \a signature.
		void publishCosignature(const model::TransactionInfo& parentTransactionInfo, const Key& signer, const Signature& signature);

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
		std::unique_ptr<model::NotificationPublisher> m_pNotificationPublisher;
		std::unique_ptr<SynchronizedPublisher> m_pSynchronizedPublisher;
	};
}}
