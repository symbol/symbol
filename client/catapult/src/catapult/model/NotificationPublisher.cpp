#include "NotificationPublisher.h"
#include "Block.h"
#include "NotificationSubscriber.h"
#include "TransactionPlugin.h"

namespace catapult { namespace model {

	namespace {
		class BasicNotificationPublisher : public NotificationPublisher {
		public:
			explicit BasicNotificationPublisher(const TransactionRegistry& transactionRegistry)
					: m_transactionRegistry(transactionRegistry)
			{}

		public:
			void publish(const WeakEntityInfoT<VerifiableEntity>& entityInfo, NotificationSubscriber& sub) const override {
				const auto& entity = entityInfo.entity();
				sub.notify(AccountPublicKeyNotification(entity.Signer));
				sub.notify(EntityNotification(entity.Network()));

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
				sub.notify(TransactionNotification(transaction.Signer, hash, transaction.Type, transaction.Deadline));

				const auto& plugin = *m_transactionRegistry.findPlugin(transaction.Type);
				sub.notify(SignatureNotification(transaction.Signer, transaction.Signature, plugin.dataBuffer(transaction)));
			}

		private:
			const TransactionRegistry& m_transactionRegistry;
		};

		class CustomNotificationPublisher : public NotificationPublisher {
		public:
			explicit CustomNotificationPublisher(const TransactionRegistry& transactionRegistry)
					: m_transactionRegistry(transactionRegistry)
			{}

		public:
			void publish(const WeakEntityInfoT<VerifiableEntity>& entityInfo, NotificationSubscriber& sub) const override {
				if (BasicEntityType::Transaction != ToBasicEntityType(entityInfo.type()))
					return;

				return publish(static_cast<const Transaction&>(entityInfo.entity()), entityInfo.hash(), sub);
			}

			void publish(const Transaction& transaction, const Hash256& hash, NotificationSubscriber& sub) const {
				const auto& plugin = *m_transactionRegistry.findPlugin(transaction.Type);
				plugin.publish(WeakEntityInfoT<model::Transaction>(transaction, hash), sub);
			}

		private:
			const TransactionRegistry& m_transactionRegistry;
		};

		class AllNotificationPublisher : public NotificationPublisher {
		public:
			explicit AllNotificationPublisher(const TransactionRegistry& transactionRegistry)
					: m_basicPublisher(transactionRegistry)
					, m_customPublisher(transactionRegistry)
			{}

		public:
			void publish(const WeakEntityInfoT<VerifiableEntity>& entityInfo, NotificationSubscriber& sub) const override {
				m_basicPublisher.publish(entityInfo, sub);
				m_customPublisher.publish(entityInfo, sub);
			}

		private:
			BasicNotificationPublisher m_basicPublisher;
			CustomNotificationPublisher m_customPublisher;
		};
	}

	std::unique_ptr<NotificationPublisher> CreateNotificationPublisher(
			const TransactionRegistry& transactionRegistry,
			PublicationMode mode) {
		switch (mode) {
		case PublicationMode::Basic:
			return std::make_unique<BasicNotificationPublisher>(transactionRegistry);

		case PublicationMode::Custom:
			return std::make_unique<CustomNotificationPublisher>(transactionRegistry);

		default:
			return std::make_unique<AllNotificationPublisher>(transactionRegistry);
		}
	}
}}
