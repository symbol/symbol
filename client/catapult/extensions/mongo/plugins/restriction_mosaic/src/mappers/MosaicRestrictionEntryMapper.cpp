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

#include "MosaicRestrictionEntryMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	// region ToDbModel

	namespace {
		void StreamAddressRestriction(bson_stream::document& builder, const state::MosaicAddressRestriction& addressRestriction) {
			builder
					<< "mosaicId" << ToInt64(addressRestriction.mosaicId())
					<< "targetAddress" << ToBinary(addressRestriction.address());

			auto restrictionArray = builder << "restrictions" << bson_stream::open_array;
			for (auto key : addressRestriction.keys()) {
				restrictionArray
						<< bson_stream::open_document
							<< "key" << static_cast<int64_t>(key)
							<< "value" << static_cast<int64_t>(addressRestriction.get(key))
						<< bson_stream::close_document;
			}

			restrictionArray << bson_stream::close_array;
		}

		void StreamGlobalRestriction(bson_stream::document& builder, const state::MosaicGlobalRestriction& globalRestriction) {
			builder << "mosaicId" << ToInt64(globalRestriction.mosaicId());

			auto restrictionArray = builder << "restrictions" << bson_stream::open_array;
			for (auto key : globalRestriction.keys()) {
				state::MosaicGlobalRestriction::RestrictionRule rule;
				globalRestriction.tryGet(key, rule);
				restrictionArray
						<< bson_stream::open_document
							<< "key" << static_cast<int64_t>(key)
							<< "restriction" << bson_stream::open_document
								<< "referenceMosaicId" << ToInt64(rule.ReferenceMosaicId)
								<< "restrictionValue" << static_cast<int64_t>(rule.RestrictionValue)
								<< "restrictionType" << utils::to_underlying_type(rule.RestrictionType)
							<< bson_stream::close_document
						<< bson_stream::close_document;
			}

			restrictionArray << bson_stream::close_array;
		}
	}

	bsoncxx::document::value ToDbModel(const state::MosaicRestrictionEntry& restrictionEntry) {
		bson_stream::document builder;
		auto doc = builder
				<< "mosaicRestrictionEntry" << bson_stream::open_document
					<< "version" << 1
					<< "compositeHash" << ToBinary(restrictionEntry.uniqueKey())
					<< "entryType" << utils::to_underlying_type(restrictionEntry.entryType());

		if (state::MosaicRestrictionEntry::EntryType::Address == restrictionEntry.entryType())
			StreamAddressRestriction(builder, restrictionEntry.asAddressRestriction());
		else
			StreamGlobalRestriction(builder, restrictionEntry.asGlobalRestriction());

		return doc
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}

	// endregion
}}}
