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

#include "AccountPropertiesMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	namespace {
		void StreamAccountProperty(bson_stream::array_context& context, const state::AccountProperty& accountProperty) {
			auto keyContext = context
					<< bson_stream::open_document
					<< "propertyType" << static_cast<int32_t>(accountProperty.descriptor().raw());

			auto valueArray = keyContext << "values" << bson_stream::open_array;
			for (const auto& value : accountProperty.values())
				valueArray << ToBinary(value.data(), value.size());

			valueArray << bson_stream::close_array;
			keyContext << bson_stream::close_document;
		}
	}

	bsoncxx::document::value ToDbModel(const state::AccountProperties& accountProperties) {
		bson_stream::document builder;
		auto doc = builder << "accountProperties" << bson_stream::open_document
				<< "address" << ToBinary(accountProperties.address());

		auto propertyArray = builder << "properties" << bson_stream::open_array;
		for (const auto& pair : accountProperties)
			StreamAccountProperty(propertyArray, pair.second);

		propertyArray << bson_stream::close_array;

		return doc
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	// endregion

	// region ToModel

	namespace {
		void ReadValues(state::AccountProperty& property, state::OperationType operationType, const bsoncxx::array::view& dbValues) {
			if (dbValues.empty())
				return;

			// note that all values have same size
			state::AccountProperty::RawPropertyValue value(dbValues.cbegin()->get_binary().size);
			for (const auto& dbValue : dbValues) {
				std::memcpy(value.data(), dbValue.get_binary().bytes, value.size());
				if (state::OperationType::Allow == operationType)
					property.allow({ model::PropertyModificationType::Add, value });
				else
					property.block({ model::PropertyModificationType::Add, value });
			}
		}
	}

	state::AccountProperties ToAccountProperties(const bsoncxx::document::view& document) {
		auto dbAccountProperties = document["accountProperties"];
		Address address;
		DbBinaryToModelArray(address, dbAccountProperties["address"].get_binary());
		state::AccountProperties accountProperties(address);

		auto dbProperties = dbAccountProperties["properties"].get_array().value;
		for (const auto& dbProperty : dbProperties) {
			auto propertyType = model::PropertyType(ToUint8(dbProperty["propertyType"].get_int32()));
			auto descriptor = state::PropertyDescriptor(propertyType);
			auto& property = accountProperties.property(descriptor.propertyType());
			ReadValues(property, descriptor.operationType(), dbProperty["values"].get_array().value);
		}

		return accountProperties;
	}

	// endregion
}}}
