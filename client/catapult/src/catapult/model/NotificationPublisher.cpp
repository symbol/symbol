#include "NotificationPublisher.h"
#include "Block.h"
#include "NotificationSubscriber.h"
#include "TransactionPlugin.h"

namespace catapult { namespace model {

	namespace {
		class DefaultNotificationPublisher : public NotificationPublisher {
		public:
			explicit DefaultNotificationPublisher(const TransactionRegistry& transactionRegistry)
					: m_transactionRegistry(transactionRegistry)
			{}

		public:
			void publish(const WeakEntityInfoT<VerifiableEntity>& entityInfo, NotificationSubscriber& sub) const override {
				const auto& entity = entityInfo.entity();
				sub.notify(AccountPublicKeyNotification(entity.Signer));
				sub.notify(EntityNotification(static_cast<NetworkIdentifier>(entity.Network())));

				switch (ToBasicEntityType(entityInfo.type())) {
				case BasicEntityType::Block:
					return publish(static_cast<const Block&>(entity), sub);

				case BasicEntityType::Transaction:
					return publish(static_cast<const Transaction&>(entity), entityInfo.hash(), sub);

				default:
					return;
				}
			}

		private:
			void publish(const Block& block, NotificationSubscriber& sub) const {
				// raise a block notification
				BlockNotification blockNotification(block.Signer, block.Timestamp, block.Difficulty);
				for (const auto& transaction : block.Transactions()) {
					blockNotification.TotalFee = blockNotification.TotalFee + transaction.Fee;
					++blockNotification.NumTransactions;
				}

				sub.notify(blockNotification);

				// raise a signature notification
				auto headerSize = VerifiableEntity::Header_Size;
				auto blockData = RawBuffer{ reinterpret_cast<const uint8_t*>(&block) + headerSize, sizeof(Block) - headerSize };
				sub.notify(SignatureNotification(block.Signer, block.Signature, blockData));
			}

			void publish(const Transaction& transaction, const Hash256& hash, NotificationSubscriber& sub) const {
				sub.notify(TransactionNotification(transaction.Signer, hash, transaction.Deadline));

				const auto* pPlugin = m_transactionRegistry.findPlugin(transaction.Type);
				sub.notify(SignatureNotification(transaction.Signer, transaction.Signature, pPlugin->dataBuffer(transaction)));
				pPlugin->publish(WeakEntityInfoT<model::Transaction>(transaction, hash), sub);
			}

		private:
			const TransactionRegistry& m_transactionRegistry;
		};
	}

	std::unique_ptr<NotificationPublisher> CreateNotificationPublisher(const TransactionRegistry& transactionRegistry) {
		return std::make_unique<DefaultNotificationPublisher>(transactionRegistry);
	}
}}
