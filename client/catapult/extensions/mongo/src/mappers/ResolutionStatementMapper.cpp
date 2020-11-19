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

#include "ResolutionStatementMapper.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		struct AddressTraits {
			using StatementType = model::AddressResolutionStatement;

			static auto ToDbValue(const UnresolvedAddress& address) {
				return ToBinary(address);
			}

			static auto ToDbValue(const Address& address) {
				return ToBinary(address);
			}
		};

		struct MosaicTraits {
			using StatementType = model::MosaicResolutionStatement;

			static auto ToDbValue(UnresolvedMosaicId mosaicId) {
				return ToInt64(mosaicId);
			}

			static auto ToDbValue(MosaicId mosaicId) {
				return ToInt64(mosaicId);
			}
		};

		template<typename TTraits>
		bsoncxx::document::value StreamResolution(Height height, const typename TTraits::StatementType& statement) {
			bson_stream::document builder;
			auto doc = builder
					<< "statement" << bson_stream::open_document
						<< "height" << ToInt64(height)
						<< "unresolved" << TTraits::ToDbValue(statement.unresolved());

			auto resolutionsArray = builder << "resolutionEntries" << bson_stream::open_array;
			for (auto i = 0u; i < statement.size(); ++i) {
				const auto& resolutionEntry = statement.entryAt(i);
				resolutionsArray
						<< bson_stream::open_document
							<< "source" << bson_stream::open_document
								<< "primaryId" << static_cast<int32_t>(resolutionEntry.Source.PrimaryId)
								<< "secondaryId" << static_cast<int32_t>(resolutionEntry.Source.SecondaryId)
							<< bson_stream::close_document
							<< "resolved" << TTraits::ToDbValue(resolutionEntry.ResolvedValue)
						<< bson_stream::close_document;
			}

			resolutionsArray << bson_stream::close_array;
			doc << bson_stream::close_document;
			return builder << bson_stream::finalize;
		}
	}

	bsoncxx::document::value ToDbModel(Height height, const model::AddressResolutionStatement& statement) {
		return StreamResolution<AddressTraits>(height, statement);
	}

	bsoncxx::document::value ToDbModel(Height height, const model::MosaicResolutionStatement& statement) {
		return StreamResolution<MosaicTraits>(height, statement);
	}
}}}
