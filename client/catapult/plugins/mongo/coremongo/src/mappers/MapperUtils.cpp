#include "MapperUtils.h"
#include "catapult/model/VerifiableEntity.h"

namespace catapult { namespace mongo { namespace mappers {

	bsoncxx::types::b_binary ToBinary(const uint8_t* pData, size_t size) {
		return { bsoncxx::binary_sub_type::k_binary, utils::checked_cast<size_t, uint32_t>(size), pData };
	}

	uint32_t ToUint32(int32_t value) {
		return static_cast<uint32_t>(value);
	}

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

	bson_stream::document& StreamEmbeddedEntity(bson_stream::document& builder, const model::EmbeddedEntity& entity) {
		return StreamBasicEntity(builder, entity);
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
}}}
