#include "NamespaceDescriptorMapper.h"
#include "plugins/mongo/coremongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/NamespaceTypes.h"
#include "plugins/txes/namespace/src/state/RootNamespace.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		constexpr auto Root_Type = utils::to_underlying_type(model::NamespaceType::Root);
		constexpr auto Child_Type = utils::to_underlying_type(model::NamespaceType::Child);

		using Path = state::Namespace::Path;

		void StreamDescriptorMetadata(bson_stream::document& builder, const state::NamespaceDescriptor& descriptor) {
			builder << "meta"
					<< bson_stream::open_document
						<< "active" << descriptor.IsActive
						<< "index" << static_cast<int32_t>(descriptor.Index)
					<< bson_stream::close_document;
		}
	}

	// region ToDbModel

	bsoncxx::document::value ToDbModel(const state::NamespaceDescriptor& descriptor) {
		const auto& path = descriptor.Path;
		const auto& root = *descriptor.pRoot;
		auto depth = path.size();
		bson_stream::document builder;
		StreamDescriptorMetadata(builder, descriptor);
		builder
				<< "namespace" << bson_stream::open_document
					<< "type" << (descriptor.IsRoot() ? Root_Type : Child_Type)
					<< "depth" << static_cast<int32_t>(path.size())
					<< "level0" << ToInt64(path[0]);

		if (1 < depth)
			builder << "level1" << ToInt64(path[1]);

		if (2 < depth)
			builder << "level2" << ToInt64(path[2]);

		builder
					<< "parentId" << ToInt64(descriptor.IsRoot() ? Namespace_Base_Id : path[path.size() - 2])
					<< "owner" << ToBinary(root.owner())
					<< "startHeight" << ToInt64(root.lifetime().Start)
					<< "endHeight" << ToInt64(root.lifetime().End)
				<< bson_stream::close_document;

		return builder << bson_stream::finalize;
	}

	// endregion

	// region ToModel

	state::NamespaceDescriptor ToNamespaceDescriptor(const bsoncxx::document::view& document) {
		// metadata
		auto dbMeta = document["meta"];
		auto isActive = dbMeta["active"].get_bool();
		auto index = static_cast<uint32_t>(dbMeta["index"].get_int32());

		// data
		auto dbNamespace = document["namespace"];

		// - path
		auto depth = dbNamespace["depth"].get_int32();
		Path path;
		path.push_back(GetValue64<NamespaceId>(dbNamespace["level0"]));
		if (1 < depth)
			path.push_back(GetValue64<NamespaceId>(dbNamespace["level1"]));

		if (2 < depth)
			path.push_back(GetValue64<NamespaceId>(dbNamespace["level2"]));

		// - root
		Key owner;
		DbBinaryToModelArray(owner, dbNamespace["owner"].get_binary());
		state::NamespaceLifetime lifetime(
				GetValue64<Height>(dbNamespace["startHeight"]),
				GetValue64<Height>(dbNamespace["endHeight"]));
		auto pRoot = std::make_shared<state::RootNamespace>(path[0], owner, lifetime);

		return state::NamespaceDescriptor(path, pRoot, index, isActive);
	}

	//endregion
}}}
