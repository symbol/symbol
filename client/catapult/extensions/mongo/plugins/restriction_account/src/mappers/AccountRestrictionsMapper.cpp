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

#include "AccountRestrictionsMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	namespace {
		void StreamAccountRestriction(bson_stream::array_context& context, const state::AccountRestriction& restriction) {
			if (restriction.values().empty())
				return;

			auto keyContext = context
					<< bson_stream::open_document
					<< "restrictionFlags" << static_cast<int32_t>(restriction.descriptor().raw());

			auto valueArray = keyContext << "values" << bson_stream::open_array;
			for (const auto& value : restriction.values())
				valueArray << ToBinary(value.data(), value.size());

			valueArray << bson_stream::close_array;
			keyContext << bson_stream::close_document;
		}
	}

	bsoncxx::document::value ToDbModel(const state::AccountRestrictions& restrictions) {
		bson_stream::document builder;
		auto doc = builder
				<< "accountRestrictions" << bson_stream::open_document
					<< "address" << ToBinary(restrictions.address());

		auto restrictionArray = builder << "restrictions" << bson_stream::open_array;
		for (const auto& pair : restrictions)
			StreamAccountRestriction(restrictionArray, pair.second);

		restrictionArray << bson_stream::close_array;

		return doc
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	// endregion
}}}
