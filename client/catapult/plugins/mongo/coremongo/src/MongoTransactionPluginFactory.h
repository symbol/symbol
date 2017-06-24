#pragma once
#include "MongoTransactionPlugin.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Factory for creating mongo transaction plugins.
	class MongoTransactionPluginFactory {
	private:
		using TransactionPlugin = MongoTransactionPlugin;
		using EmbeddedTransactionPlugin = EmbeddedMongoTransactionPlugin;

	public:
		/// Creates an embedded transaction plugin around \a streamEmbeddedFunc.
		template<typename TEmbeddedTransaction, typename TStreamEmbeddedFunc>
		static std::unique_ptr<EmbeddedTransactionPlugin> CreateEmbedded(TStreamEmbeddedFunc streamEmbeddedFunc) {
			return std::make_unique<EmbeddedTransactionPluginT<TEmbeddedTransaction>>(streamEmbeddedFunc);
		}

		/// Creates a transaction plugin that supports embedding around \a streamFunc and \a streamEmbeddedFunc.
		template<typename TTransaction, typename TEmbeddedTransaction, typename TStreamFunc, typename TStreamEmbeddedFunc>
		static std::unique_ptr<TransactionPlugin> Create(TStreamFunc streamFunc, TStreamEmbeddedFunc streamEmbeddedFunc) {
			return std::make_unique<TransactionPluginT<TTransaction, TEmbeddedTransaction>>(streamFunc, streamEmbeddedFunc);
		}

	private:
		template<typename TTransaction, typename TDerivedTransaction, typename TPlugin>
		class BasicTransactionPluginT : public TPlugin {
		private:
			using StreamFunc = std::function<void (bsoncxx::builder::stream::document&, const TDerivedTransaction&)>;

		public:
			explicit BasicTransactionPluginT(const StreamFunc& streamFunc) : m_streamFunc(streamFunc)
			{}

		public:
			model::EntityType type() const override {
				return TDerivedTransaction::Entity_Type;
			}

			void streamTransaction(bsoncxx::builder::stream::document& builder, const TTransaction& transaction) const override {
				m_streamFunc(builder, static_cast<const TDerivedTransaction&>(transaction));
			}

		private:
			StreamFunc m_streamFunc;
		};

		template<typename TEmbeddedTransaction>
		using EmbeddedTransactionPluginT = BasicTransactionPluginT<model::EmbeddedEntity, TEmbeddedTransaction, EmbeddedTransactionPlugin>;

		template<typename TTransaction, typename TEmbeddedTransaction>
		class TransactionPluginT : public BasicTransactionPluginT<model::Transaction, TTransaction, TransactionPlugin> {
		public:
			template<typename TStreamFunc, typename TStreamEmbeddedFunc>
			explicit TransactionPluginT(TStreamFunc streamFunc, TStreamEmbeddedFunc streamEmbeddedFunc)
					: BasicTransactionPluginT<model::Transaction, TTransaction, TransactionPlugin>(streamFunc)
					, m_pEmbeddedTransactionPlugin(CreateEmbedded<TEmbeddedTransaction>(streamEmbeddedFunc))
			{}

		public:
			std::vector<bsoncxx::document::value> extractDependentDocuments(
					const model::Transaction&,
					const MongoTransactionMetadata&) const override {
				// don't support any dependent documents by default
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
}}}
