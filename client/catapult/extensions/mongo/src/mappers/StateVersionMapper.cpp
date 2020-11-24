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

#include "StateVersionMapper.h"
#include "MapperUtils.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		constexpr auto Meta_Document_Name = "meta";
		constexpr auto Version_Field_Name = "version";

		bsoncxx::document::value AddMetaSubDocument(const bsoncxx::document::value& doc, uint16_t version) {
			bson_stream::document builder;
			return builder
					<< std::string(Meta_Document_Name) << bson_stream::open_document
						<< std::string(Version_Field_Name) << version
					<< bson_stream::close_document
					<< bsoncxx::builder::concatenate(doc.view())
					<< bson_stream::finalize;
		}

		bsoncxx::document::value SetMetaVersion(const bsoncxx::document::value& doc, uint16_t version) {
			bson_stream::document builder;
			for (const auto& element : doc.view()) {
				if (Meta_Document_Name != element.key()) {
					builder << element.key() << element.get_value();
				} else {
					builder
							<< std::string(Meta_Document_Name) << bson_stream::open_document
								<< std::string(Version_Field_Name) << version;

					for (const auto& metaElement : element.get_document().view()) {
						if (Version_Field_Name != metaElement.key())
							builder << metaElement.key() << metaElement.get_value();
					}

					builder << bson_stream::close_document;
				}
			}

			return builder << bson_stream::finalize;
		}
	}

	bsoncxx::document::value AddStateVersion(const bsoncxx::document::value& doc, uint16_t version) {
		return !doc.view()[Meta_Document_Name]
				? AddMetaSubDocument(doc, version)
				: SetMetaVersion(doc, version);
	}
}}}
