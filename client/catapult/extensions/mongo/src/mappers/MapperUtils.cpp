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

#include "MapperUtils.h"
#include "catapult/model/EmbeddedTransaction.h"
#include "catapult/model/Receipt.h"
#include "catapult/model/VerifiableEntity.h"

namespace catapult { namespace mongo { namespace mappers {

	// region conversions to db type

	bsoncxx::types::b_binary ToBinary(const uint8_t* pData, size_t size) {
		return { bsoncxx::binary_sub_type::k_binary, utils::checked_cast<size_t, uint32_t>(size), pData };
	}

	bsoncxx::types::b_binary ToBinary(const UnresolvedAddress& unresolvedAddress) {
		return ToBinary(reinterpret_cast<const uint8_t*>(unresolvedAddress.data()), unresolvedAddress.size());
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
					<< "signerPublicKey" << ToBinary(entity.SignerPublicKey)
					<< "version" << entity.Version
					<< "network" << utils::to_underlying_type(entity.Network)
					<< "type" << utils::to_underlying_type(entity.Type);
			return builder;
		}
	}

	bson_stream::document& StreamEmbeddedTransaction(bson_stream::document& builder, const model::EmbeddedTransaction& transaction) {
		return StreamBasicEntity(builder, transaction);
	}

	bson_stream::document& StreamVerifiableEntity(bson_stream::document& builder, const model::VerifiableEntity& entity) {
		builder
				<< "size" << static_cast<int32_t>(entity.Size)
				<< "signature" << ToBinary(entity.Signature);
		return StreamBasicEntity(builder, entity);
	}

	namespace {
		template<typename TMosaicId>
		bson_stream::array_context& StreamMosaicT(bson_stream::array_context& context, TMosaicId id, Amount amount) {
			context
					<< bson_stream::open_document
						<< "id" << ToInt64(id)
						<< "amount" << ToInt64(amount)
					<< bson_stream::close_document;
			return context;
		}
	}

	bson_stream::array_context& StreamMosaic(bson_stream::array_context& context, MosaicId id, Amount amount) {
		return StreamMosaicT(context, id, amount);
	}

	bson_stream::array_context& StreamMosaic(bson_stream::array_context& context, UnresolvedMosaicId id, Amount amount) {
		return StreamMosaicT(context, id, amount);
	}

	bson_stream::document& StreamReceipt(bson_stream::document& builder, const model::Receipt& receipt) {
		builder
				<< "version" << receipt.Version
				<< "type" << utils::to_underlying_type(receipt.Type);

		return builder;
	}

	// endregion
}}}
