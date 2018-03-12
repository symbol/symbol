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
					<< "$push"
						<< open_document
							<< "transaction.cosignatures"
							<< open_document
								<< "$each"
								<< open_array;

			for (const model::Cosignature& cosignature : cosignatures) {
				array
						<< open_document
							<< "signer" << mappers::ToBinary(cosignature.Signer)
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
			explicit DefaultMongoPtStorage(MongoStorageContext& context, const MongoTransactionRegistry& transactionRegistry)
					: m_pTransactionStorage(CreateMongoTransactionStorage(context, transactionRegistry, Pt_Collection_Name))
					, m_database(context.createDatabaseConnection())
			{}

		public:
			void notifyAddPartials(const TransactionInfos& transactionInfos) override {
				m_pTransactionStorage->notifyAdds(transactionInfos);
			}

			void notifyAddCosignature(
					const model::TransactionInfo& parentTransactionInfo,
					const Key& signer,
					const Signature& signature) override {
				// this function is only called by the pt cache modifier if parentInfo corresponds to a known partial transaction
				auto& cosignatures = m_cosignaturesMap[parentTransactionInfo.EntityHash];
				cosignatures.push_back({ signer, signature });
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
