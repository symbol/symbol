#include "MapperTestUtils.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/NamespaceTypes.h"
#include "plugins/txes/namespace/src/state/MosaicDescriptor.h"
#include "plugins/txes/namespace/src/state/NamespaceDescriptor.h"
#include "plugins/txes/namespace/src/state/RootNamespace.h"
#include "catapult/utils/Casting.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::test;

namespace catapult { namespace mongo { namespace test {

	// region mosaic related

	namespace {
		namespace test = catapult::test;

		void AssertMosaicProperties(const model::MosaicProperties& properties, const bsoncxx::document::view& dbProperties) {
			ASSERT_EQ(properties.size(), std::distance(dbProperties.cbegin(), dbProperties.cend()));
			auto dbIter = dbProperties.cbegin();
			for (auto iter = properties.cbegin(); properties.cend() != iter; ++iter) {
				EXPECT_EQ(iter->Value, static_cast<uint64_t>(dbIter->get_int64().value));
				++dbIter;
			}
		}
	}

	void AssertEqualMosaicMetadata(const state::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMetadata) {
		EXPECT_EQ(descriptor.IsActive, dbMetadata["active"].get_bool().value);
		EXPECT_EQ(descriptor.Index, GetUint32(dbMetadata, "index"));
	}

	void AssertEqualMosaicData(const state::MosaicDescriptor& descriptor, const bsoncxx::document::view& dbMosaic) {
		const auto& entry = *descriptor.pEntry;
		EXPECT_EQ(entry.namespaceId(), NamespaceId(GetUint64(dbMosaic, "namespaceId")));
		EXPECT_EQ(entry.mosaicId(), MosaicId(GetUint64(dbMosaic, "mosaicId")));
		EXPECT_EQ(entry.supply(), Amount(GetUint64(dbMosaic, "supply")));

		const auto& definition = entry.definition();
		EXPECT_EQ(definition.height(), Height(GetUint64(dbMosaic, "height")));
		EXPECT_EQ(ToHexString(definition.owner()), ToHexString(GetBinary(dbMosaic, "owner"), Key_Size));

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

	void AssertEqualNamespaceMetadata(const state::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbMetadata) {
		EXPECT_EQ(descriptor.IsActive, dbMetadata["active"].get_bool().value);
		EXPECT_EQ(descriptor.Index, GetUint32(dbMetadata, "index"));
	}

	void AssertEqualNamespaceData(const state::NamespaceDescriptor& descriptor, const bsoncxx::document::view& dbNamespace) {
		auto depth = descriptor.Path.size();
		auto isRoot = 1 == depth;
		EXPECT_EQ(isRoot ? Root_Type : Child_Type, GetUint32(dbNamespace, "type"));
		EXPECT_EQ(depth, GetUint32(dbNamespace, "depth"));
		EXPECT_EQ(descriptor.Path[0], NamespaceId(GetUint64(dbNamespace, "level0")));

		if (1 < depth)
			EXPECT_EQ(descriptor.Path[1], NamespaceId(GetUint64(dbNamespace, "level1")));

		if (2 < depth)
			EXPECT_EQ(descriptor.Path[2], NamespaceId(GetUint64(dbNamespace, "level2")));

		EXPECT_EQ(isRoot ? Namespace_Base_Id : descriptor.Path[depth - 2], NamespaceId(GetUint64(dbNamespace, "parentId")));
		EXPECT_EQ(ToHexString(descriptor.pRoot->owner()), ToHexString(GetBinary(dbNamespace, "owner"), Key_Size));
		EXPECT_EQ(descriptor.pRoot->lifetime().Start, Height(GetUint64(dbNamespace, "startHeight")));
		EXPECT_EQ(descriptor.pRoot->lifetime().End, Height(GetUint64(dbNamespace, "endHeight")));
	}

	// endregion
}}}
