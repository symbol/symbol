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
#include "MongoTransactionPlugin.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace mongo {

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
			using StreamFunc = consumer<bsoncxx::builder::stream::document&, const TDerivedTransaction&>;

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
		using EmbeddedTransactionPluginT = BasicTransactionPluginT<
			model::EmbeddedTransaction,
			TEmbeddedTransaction,
			EmbeddedTransactionPlugin>;

		template<typename TTransaction, typename TEmbeddedTransaction>
		class TransactionPluginT : public BasicTransactionPluginT<model::Transaction, TTransaction, TransactionPlugin> {
		public:
			template<typename TStreamFunc, typename TStreamEmbeddedFunc>
			TransactionPluginT(TStreamFunc streamFunc, TStreamEmbeddedFunc streamEmbeddedFunc)
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

/// Defines a mongo transaction plugin factory for \a NAME transaction using \a STREAM.
#define DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(NAME, STREAM) \
	std::unique_ptr<MongoTransactionPlugin> Create##NAME##TransactionMongoPlugin() { \
		return MongoTransactionPluginFactory::Create<model::NAME##Transaction, model::Embedded##NAME##Transaction>( \
				STREAM<model::NAME##Transaction>, \
				STREAM<model::Embedded##NAME##Transaction>); \
	}
}}
