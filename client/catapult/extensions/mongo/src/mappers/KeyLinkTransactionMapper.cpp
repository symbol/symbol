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

#include "KeyLinkTransactionMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/coresystem/src/model/VotingKeyLinkTransaction.h"
#include "plugins/coresystem/src/model/VrfKeyLinkTransaction.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		template<typename TTransaction>
		void StreamVotingTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			if (1 == transaction.Version) {
				std::array<uint8_t, VotingKey::Size + 16> v1VotingKey{};
				std::memcpy(v1VotingKey.data(), transaction.LinkedPublicKey.data(), VotingKey::Size);
				builder << "linkedPublicKey" << ToBinary(v1VotingKey.data(), v1VotingKey.size());
			} else {
				builder << "linkedPublicKey" << ToBinary(transaction.LinkedPublicKey);
			}

			builder
					<< "startEpoch" << ToInt32(transaction.StartEpoch)
					<< "endEpoch" << ToInt32(transaction.EndEpoch)
					<< "linkAction" << utils::to_underlying_type(transaction.LinkAction);
		}

		class MongoVotingTransactionPlugin : public MongoTransactionPlugin {
		public:
			model::EntityType type() const override {
				return model::Entity_Type_Voting_Key_Link;
			}

			void streamTransaction(bsoncxx::builder::stream::document& builder, const model::Transaction& transaction) const override {
				if (transaction.Version > 1)
					StreamVotingTransaction(builder, static_cast<const model::VotingKeyLinkTransaction&>(transaction));
				else
					StreamVotingTransaction(builder, static_cast<const model::VotingKeyLinkV1Transaction&>(transaction));
			}

			std::vector<bsoncxx::document::value> extractDependentDocuments(
					const model::Transaction&,
					const MongoTransactionMetadata&) const override {
				return {};
			}

			bool supportsEmbedding() const override {
				return true;
			}

			const EmbeddedMongoTransactionPlugin& embeddedPlugin() const override {
				return m_embeddedPlugin;
			}

		private:
			class MongoVotingTransactionEmbeddedPlugin : public EmbeddedMongoTransactionPlugin {
			public:
				model::EntityType type() const override {
					return model::Entity_Type_Voting_Key_Link;
				}

				void streamTransaction(
						bsoncxx::builder::stream::document& builder,
						const model::EmbeddedTransaction& transaction) const override {
					if (transaction.Version > 1)
						StreamVotingTransaction(builder, static_cast<const model::EmbeddedVotingKeyLinkTransaction&>(transaction));
					else
						StreamVotingTransaction(builder, static_cast<const model::EmbeddedVotingKeyLinkV1Transaction&>(transaction));
				}
			};

		private:
			MongoVotingTransactionEmbeddedPlugin m_embeddedPlugin;
		};
	}

	std::unique_ptr<MongoTransactionPlugin> CreateVotingKeyLinkTransactionMongoPlugin() {
		return std::make_unique<MongoVotingTransactionPlugin>();
	}

	namespace {
		template<typename TTransaction>
		void StreamVrfTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "linkedPublicKey" << ToBinary(transaction.LinkedPublicKey)
					<< "linkAction" << utils::to_underlying_type(transaction.LinkAction);
		}
	}

	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(VrfKeyLink, StreamVrfTransaction)
}}}
