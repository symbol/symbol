#pragma once
#include "TransactionPlugin.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

	/// Factory for creating transaction plugins.
	class TransactionPluginFactory {
	public:
		/// Creates an embedded transaction plugin around \a publishEmbeddedFunc.
		template<typename TEmbeddedTransaction, typename TPublishEmbeddedFunc>
		static std::unique_ptr<EmbeddedTransactionPlugin> CreateEmbedded(TPublishEmbeddedFunc publishEmbeddedFunc) {
			return std::make_unique<EmbeddedTransactionPluginT<TEmbeddedTransaction>>(publishEmbeddedFunc);
		}

		/// Creates a transaction plugin that supports embedding around \a publishFunc and \a publishEmbeddedFunc.
		template<typename TTransaction, typename TEmbeddedTransaction, typename TPublishFunc, typename TPublishEmbeddedFunc>
		static std::unique_ptr<TransactionPlugin> Create(TPublishFunc publishFunc, TPublishEmbeddedFunc publishEmbeddedFunc) {
			return std::make_unique<TransactionPluginT<TTransaction, TEmbeddedTransaction>>(publishFunc, publishEmbeddedFunc);
		}

	private:
		template<typename TTransaction, typename TDerivedTransaction, typename TPlugin>
		class BasicTransactionPluginT : public TPlugin {
		private:
			using PublishFunc = std::function<void (const TDerivedTransaction&, NotificationSubscriber&)>;

		public:
			explicit BasicTransactionPluginT(const PublishFunc& publishFunc) : m_publishFunc(publishFunc)
			{}

		public:
			EntityType type() const override {
				return TDerivedTransaction::Entity_Type;
			}

			uint64_t calculateRealSize(const TTransaction& transaction) const override {
				return TDerivedTransaction::CalculateRealSize(static_cast<const TDerivedTransaction&>(transaction));
			}

		protected:
			void publishImpl(const TTransaction& transaction, NotificationSubscriber& sub) const {
				m_publishFunc(static_cast<const TDerivedTransaction&>(transaction), sub);
			}

		private:
			PublishFunc m_publishFunc;
		};

		template<typename TEmbeddedTransaction>
		class EmbeddedTransactionPluginT
				: public BasicTransactionPluginT<EmbeddedEntity, TEmbeddedTransaction, EmbeddedTransactionPlugin> {
		private:
			using BaseType = BasicTransactionPluginT<EmbeddedEntity, TEmbeddedTransaction, EmbeddedTransactionPlugin>;

		public:
			template<typename TPublishEmbeddedFunc>
			explicit EmbeddedTransactionPluginT(TPublishEmbeddedFunc publishEmbeddedFunc) : BaseType(publishEmbeddedFunc)
			{}

		public:
			void publish(const EmbeddedEntity& transaction, NotificationSubscriber& sub) const override {
				BaseType::publishImpl(transaction, sub);
			}
		};

		template<typename TTransaction, typename TEmbeddedTransaction>
		class TransactionPluginT : public BasicTransactionPluginT<Transaction, TTransaction, TransactionPlugin> {
		private:
			using BaseType = BasicTransactionPluginT<Transaction, TTransaction, TransactionPlugin>;

		public:
			template<typename TPublishFunc, typename TPublishEmbeddedFunc>
			explicit TransactionPluginT(TPublishFunc publishFunc, TPublishEmbeddedFunc publishEmbeddedFunc)
					: BaseType(publishFunc)
					, m_pEmbeddedTransactionPlugin(CreateEmbedded<TEmbeddedTransaction>(publishEmbeddedFunc))
			{}

		public:
			void publish(const WeakEntityInfoT<Transaction>& transactionInfo, NotificationSubscriber& sub) const override {
				BaseType::publishImpl(transactionInfo.entity(), sub);
			}

			RawBuffer dataBuffer(const Transaction& transaction) const override {
				auto headerSize = VerifiableEntity::Header_Size;
				return { reinterpret_cast<const uint8_t*>(&transaction) + headerSize, transaction.Size - headerSize };
			}

			std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction&) const override {
				return {};
			}

			bool supportsEmbedding() const override {
				return true;
			}

			const EmbeddedTransactionPlugin& embeddedPlugin() const override {
				return *m_pEmbeddedTransactionPlugin;
			}

		private:
			std::unique_ptr<EmbeddedTransactionPlugin> m_pEmbeddedTransactionPlugin;
		};
	};
}}
