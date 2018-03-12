#include "MapperUtils.h"
#include "catapult/model/EmbeddedTransaction.h"
#include "catapult/model/VerifiableEntity.h"

namespace catapult { namespace mongo { namespace mappers {

	// region conversions to db type

	bsoncxx::types::b_binary ToBinary(const uint8_t* pData, size_t size) {
		return { bsoncxx::binary_sub_type::k_binary, utils::checked_cast<size_t, uint32_t>(size), pData };
	}

	// endregion

	// region conversions from db type

	uint8_t ToUint8(int32_t value) {
		return utils::checked_cast<int32_t, uint8_t>(value);
	}

	uint32_t ToUint32(int32_t value) {
		return static_cast<uint32_t>(value);
	}

	// endregion

	// region document helpers

	uint64_t GetUint64OrDefault(const bsoncxx::document::view& documentView, const char* name, uint64_t defaultValue) {
		auto iter = documentView.find(name);
		if (documentView.end() == iter)
			return defaultValue;

		return static_cast<uint64_t>(iter->get_int64().value);
	}

	bool IsEmptyDocument(const bsoncxx::document::value& document) {
		return 0 == document.view().length();
	}

	// endregion

	// region streaming helpers

	namespace {
		template<typename TEntity>
		bson_stream::document& StreamBasicEntity(bson_stream::document& builder, const TEntity& entity) {
			builder
					<< "signer" << ToBinary(entity.Signer)
					<< "version" << entity.Version
					<< "type" << utils::to_underlying_type(entity.Type);
			return builder;
		}
	}

	bson_stream::document& StreamEmbeddedTransaction(bson_stream::document& builder, const model::EmbeddedTransaction& transaction) {
		return StreamBasicEntity(builder, transaction);
	}

	bson_stream::document& StreamVerifiableEntity(bson_stream::document& builder, const model::VerifiableEntity& entity) {
		builder << "signature" << ToBinary(entity.Signature);
		return StreamBasicEntity(builder, entity);
	}

	bson_stream::array_context& StreamMosaic(bson_stream::array_context& context, MosaicId id, Amount amount) {
		context
				<< bson_stream::open_document
					<< "id" << ToInt64(id)
					<< "amount" << ToInt64(amount)
				<< bson_stream::close_document;
		return context;
	}

	// endregion
}}}
