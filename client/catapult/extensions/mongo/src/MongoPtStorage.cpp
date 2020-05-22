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

#include "MongoPtStorage.h"
#include "MongoTransactionStorage.h"
#include "mappers/MapperUtils.h"
#include "mappers/TransactionMapper.h"
#include "catapult/model/Cosignature.h"
#include "catapult/utils/HexFormatter.h"
#include <unordered_map>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

	namespace {
		constexpr auto Pt_Collection_Name = "partialTransactions";

		using CosignaturesMap = std::unordered_map<Hash256, std::vector<model::Cosignature>, utils::ArrayHasher<Hash256>>;

		auto CreateFilter(const Hash256& hash) {
			return document() << "meta.hash" << mappers::ToBinary(hash) << finalize;
		}

		auto CreateAppendDocument(const std::vector<model::Cosignature>& cosignatures) {
			document doc{};
			auto array = doc
					<< "$push" << open_document
						<< "transaction.cosignatures" << open_document
							<< "$each" << open_array;

			for (const model::Cosignature& cosignature : cosignatures) {
				array
						<< open_document
							<< "version" << static_cast<int64_t>(cosignature.Version)
							<< "signerPublicKey" << mappers::ToBinary(cosignature.SignerPublicKey)
							<< "signature" << mappers::ToBinary(cosignature.Signature)
						<< close_document;
			}

			array
					<< close_array
					<< close_document
					<< close_document;
			return doc << finalize;
		}

		void FlushCosignatures(MongoDatabase& database, const CosignaturesMap& cosignaturesMap, const std::string& collectionName) {
			// if the transaction corresponding to a cosignature has been removed, update_one will have no effect
			auto collection = database[collectionName];
			for (const auto& pair : cosignaturesMap)
				collection.update_one(CreateFilter(pair.first), CreateAppendDocument(pair.second));
		}

		class DefaultMongoPtStorage final : public cache::PtChangeSubscriber {
		public:
			DefaultMongoPtStorage(MongoStorageContext& context, const MongoTransactionRegistry& transactionRegistry)
					: m_pTransactionStorage(CreateMongoTransactionStorage(context, transactionRegistry, Pt_Collection_Name))
					, m_database(context.createDatabaseConnection())
			{}

		public:
			void notifyAddPartials(const TransactionInfos& transactionInfos) override {
				m_pTransactionStorage->notifyAdds(transactionInfos);
			}

			void notifyAddCosignature(
					const model::TransactionInfo& parentTransactionInfo,
					const model::Cosignature& cosignature) override {
				// this function is only called by the pt cache modifier if parentInfo corresponds to a known partial transaction
				auto& cosignatures = m_cosignaturesMap[parentTransactionInfo.EntityHash];
				cosignatures.push_back(cosignature);
			}

			void notifyRemovePartials(const TransactionInfos& transactionInfos) override {
				m_pTransactionStorage->notifyRemoves(transactionInfos);
			}

			void flush() override {
				FlushCosignatures(m_database, m_cosignaturesMap, Pt_Collection_Name);
				m_cosignaturesMap.clear();
			}

		private:
			std::unique_ptr<cache::UtChangeSubscriber> m_pTransactionStorage;
			MongoDatabase m_database;
			CosignaturesMap m_cosignaturesMap;
		};
	}

	std::unique_ptr<cache::PtChangeSubscriber> CreateMongoPtStorage(
			MongoStorageContext& context,
			const MongoTransactionRegistry& transactionRegistry) {
		return std::make_unique<DefaultMongoPtStorage>(context, transactionRegistry);
	}
}}
