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

#include "MosaicDefinitionMapper.h"
#include "mongo/src/MongoTransactionPluginFactory.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/mosaic/src/model/MosaicDefinitionTransaction.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		void StreamProperty(bson_stream::array_context& context, model::MosaicPropertyId id, uint64_t value) {
			context << bson_stream::open_document
					<< "id" << utils::to_underlying_type(id)
					<< "value" << static_cast<int64_t>(value)
				<< bson_stream::close_document;
		}

		void StreamRequiredProperties(bson_stream::array_context& context, const model::MosaicPropertiesHeader& propertiesHeader) {
			StreamProperty(context, model::MosaicPropertyId::Flags, utils::to_underlying_type(propertiesHeader.Flags));
			StreamProperty(context, model::MosaicPropertyId::Divisibility, propertiesHeader.Divisibility);
		}

		void StreamOptionalProperties(bson_stream::array_context& context, const model::MosaicProperty* pProperty, size_t numProperties) {
			for (auto i = 0u; i < numProperties; ++i, ++pProperty)
				StreamProperty(context, pProperty->Id, pProperty->Value);
		}

		template<typename TTransaction>
		void StreamTransaction(bson_stream::document& builder, const TTransaction& transaction) {
			builder
					<< "mosaicNonce" << ToInt32(transaction.MosaicNonce)
					<< "mosaicId" << ToInt64(transaction.MosaicId);
			auto propertiesArray = builder << "properties" << bson_stream::open_array;
			StreamRequiredProperties(propertiesArray, transaction.PropertiesHeader);
			StreamOptionalProperties(propertiesArray, transaction.PropertiesPtr(), transaction.PropertiesHeader.Count);
			propertiesArray << bson_stream::close_array;
		}
	}

	DEFINE_MONGO_TRANSACTION_PLUGIN_FACTORY(MosaicDefinition, StreamTransaction)
}}}
