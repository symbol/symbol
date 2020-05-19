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

#include "NamespaceDescriptorMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/model/NamespaceTypes.h"
#include "plugins/txes/namespace/src/state/RootNamespace.h"
#include "catapult/utils/Casting.h"

using namespace catapult::mongo::mappers;

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		constexpr auto Root_Type = utils::to_underlying_type(model::NamespaceRegistrationType::Root);
		constexpr auto Child_Type = utils::to_underlying_type(model::NamespaceRegistrationType::Child);

		using Path = state::Namespace::Path;

		void StreamDescriptorMetadata(bson_stream::document& builder, const NamespaceDescriptor& descriptor) {
			builder
					<< "meta" << bson_stream::open_document
						<< "active" << descriptor.IsActive
						<< "index" << static_cast<int32_t>(descriptor.Index)
					<< bson_stream::close_document;
		}

		void StreamAlias(bson_stream::document& builder, const state::NamespaceAlias& alias) {
			builder
					<< "alias" << bson_stream::open_document
						<< "type" << utils::to_underlying_type(alias.type());

			switch (alias.type()) {
			case state::AliasType::Mosaic:
				builder << "mosaicId" << ToInt64(alias.mosaicId());
				break;

			case state::AliasType::Address:
				builder << "address" << ToBinary(alias.address());
				break;

			default:
				break;
			}

			builder << bson_stream::close_document;
		}
	}

	// region ToDbModel

	bsoncxx::document::value ToDbModel(const NamespaceDescriptor& descriptor) {
		const auto& path = descriptor.Path;
		const auto& root = *descriptor.pRoot;
		auto depth = path.size();
		bson_stream::document builder;
		StreamDescriptorMetadata(builder, descriptor);
		builder
				<< "namespace" << bson_stream::open_document
					<< "registrationType" << (descriptor.IsRoot() ? Root_Type : Child_Type)
					<< "depth" << static_cast<int32_t>(path.size());

		for (auto i = 0u; i < depth; ++i)
			builder << "level" + std::to_string(i) << ToInt64(path[i]);

		StreamAlias(builder, descriptor.Alias);

		builder
					<< "parentId" << ToInt64(descriptor.IsRoot() ? Namespace_Base_Id : path[path.size() - 2])
					<< "ownerAddress" << ToBinary(descriptor.OwnerAddress)
					<< "startHeight" << ToInt64(root.lifetime().Start)
					<< "endHeight" << ToInt64(root.lifetime().End)
				<< bson_stream::close_document;

		return builder << bson_stream::finalize;
	}

	// endregion
}}}
