#include "TransferMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/transfer/src/model/TransferTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		void StreamMessage(bson_stream::document& builder, const uint8_t* pMessage, size_t messageSize) {
			if (0 == messageSize)
				return;

			builder << "message"
					<< bson_stream::open_document
						<< "type" << pMessage[0]
						// cannot pass nullptr when 1 == messageSize because libbson asserts that data pointer is not null
						// however, pointer can be garbage because library does a memcpy with size 0
						// so pointer will not be dereferenced
						<< "payload" << ToBinary(pMessage + 1, messageSize - 1)
					<< bson_stream::close_document;
		}

		void StreamMosaics(bson_stream::document& builder, const model::Mosaic* pMosaic, size_t numMosaics) {
			auto mosaicsArray = builder << "mosaics" << bson_stream::open_array;
			for (auto i = 0u; i < numMosaics; ++i) {
				StreamMosaic(mosaicsArray, pMosaic->MosaicId, pMosaic->Amount);
				++pMosaic;
			}

			mosaicsArray << bson_stream::close_array;
		}

		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder << "recipient" << ToBinary(transaction.Recipient);
			StreamMessage(builder, transaction.MessagePtr(), transaction.MessageSize);
			StreamMosaics(builder, transaction.MosaicsPtr(), transaction.MosaicsCount);
		}
	}

	std::unique_ptr<MongoTransactionPlugin> CreateTransferTransactionMongoPlugin() {
		return MongoTransactionPluginFactory::Create<model::TransferTransaction, model::EmbeddedTransferTransaction>(
				StreamTransaction<model::TransferTransaction>,
				StreamTransaction<model::EmbeddedTransferTransaction>);
	}
}}}
