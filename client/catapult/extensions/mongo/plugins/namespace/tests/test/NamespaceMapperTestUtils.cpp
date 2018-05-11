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

#include "NamespaceMapperTestUtils.h"
#include "src/mappers/MosaicDescriptor.h"
#include "src/mappers/NamespaceDescriptor.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/NamespaceTypes.h"
#include "plugins/txes/namespace/src/state/RootNamespace.h"
#include "catapult/utils/Casting.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region mosaic related

	namespace {
		void AssertMosaicProperties(const model::MosaicProperties& properties, const bsoncxx::document::view& dbProperties) {
			ASSERT_EQ(properties.size(), std::distance(dbProperties.cbegin(), dbProperties.cend()));
			auto dbIter = dbProperties.cbegin();
			for (const auto& property : properties) {
				EXPECT_EQ(property.Value, static_cast<uint64_t>(dbIter->get_int64().value));
				++dbIter;
			}
		}
	}

	void AssertEqualMosaicMetadata(const mongo::plugins::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMetadata) {
		EXPECT_EQ(descriptor.IsActive, dbMetadata["active"].get_bool().value);
		EXPECT_EQ(descriptor.Index, GetUint32(dbMetadata, "index"));
	}

	void AssertEqualMosaicData(const mongo::plugins::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMosaic) {
		const auto& entry = *descriptor.pEntry;
		EXPECT_EQ(entry.namespaceId(), NamespaceId(GetUint64(dbMosaic, "namespaceId")));
		EXPECT_EQ(entry.mosaicId(), MosaicId(GetUint64(dbMosaic, "mosaicId")));
		EXPECT_EQ(entry.supply(), Amount(GetUint64(dbMosaic, "supply")));

		const auto& definition = entry.definition();
		EXPECT_EQ(definition.height(), Height(GetUint64(dbMosaic, "height")));
		EXPECT_EQ(definition.owner(), GetKeyValue(dbMosaic, "owner"));

		auto dbProperties = dbMosaic["properties"].get_array().value;
		const auto& properties = definition.properties();

		// two required and one optional property
		EXPECT_EQ(3u, test::GetFieldCount(dbProperties));
		AssertMosaicProperties(properties, dbProperties);

		auto dbLevy = dbMosaic["levy"].get_document().view();
		EXPECT_EQ(0u, test::GetFieldCount(dbLevy));
		EXPECT_TRUE(dbLevy.empty()); // fix when document contains levy
	}

	// endregion

	// region namespace related

	namespace {
		constexpr auto Root_Type = utils::to_underlying_type(model::NamespaceType::Root);
		constexpr auto Child_Type = utils::to_underlying_type(model::NamespaceType::Child);
	}

	void AssertEqualNamespaceMetadata(const mongo::plugins::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbMetadata) {
		EXPECT_EQ(descriptor.IsActive, dbMetadata["active"].get_bool().value);
		EXPECT_EQ(descriptor.Index, GetUint32(dbMetadata, "index"));
	}

	void AssertEqualNamespaceData(const mongo::plugins::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbNamespace) {
		auto depth = descriptor.Path.size();
		auto isRoot = 1 == depth;
		EXPECT_EQ(isRoot ? Root_Type : Child_Type, GetUint32(dbNamespace, "type"));
		EXPECT_EQ(depth, GetUint32(dbNamespace, "depth"));

		for (auto level = 0u; level < depth; ++level)
			EXPECT_EQ(descriptor.Path[level], NamespaceId(GetUint64(dbNamespace, "level" + std::to_string(level)))) << "level " << level;

		EXPECT_EQ(isRoot ? Namespace_Base_Id : descriptor.Path[depth - 2], NamespaceId(GetUint64(dbNamespace, "parentId")));
		EXPECT_EQ(descriptor.OwnerAddress, GetAddressValue(dbNamespace, "ownerAddress"));
		EXPECT_EQ(descriptor.pRoot->owner(), GetKeyValue(dbNamespace, "owner"));
		EXPECT_EQ(descriptor.pRoot->lifetime().Start, Height(GetUint64(dbNamespace, "startHeight")));
		EXPECT_EQ(descriptor.pRoot->lifetime().End, Height(GetUint64(dbNamespace, "endHeight")));
	}

	// endregion
}}
