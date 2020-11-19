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

#pragma once
#include "MetadataBaseSets.h"
#include "MetadataCacheSerializers.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the metadata cache view.
	using MetadataCacheViewMixins = PatriciaTreeCacheMixins<MetadataCacheTypes::PrimaryTypes::BaseSetType, MetadataCacheDescriptor>;

	/// Basic view on top of the metadata cache.
	class BasicMetadataCacheView
			: public utils::MoveOnly
			, public MetadataCacheViewMixins::Size
			, public MetadataCacheViewMixins::Contains
			, public MetadataCacheViewMixins::Iteration
			, public MetadataCacheViewMixins::ConstAccessor
			, public MetadataCacheViewMixins::PatriciaTreeView {
	public:
		using ReadOnlyView = MetadataCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a metadataSets.
		explicit BasicMetadataCacheView(const MetadataCacheTypes::BaseSets& metadataSets)
				: MetadataCacheViewMixins::Size(metadataSets.Primary)
				, MetadataCacheViewMixins::Contains(metadataSets.Primary)
				, MetadataCacheViewMixins::Iteration(metadataSets.Primary)
				, MetadataCacheViewMixins::ConstAccessor(metadataSets.Primary)
				, MetadataCacheViewMixins::PatriciaTreeView(metadataSets.PatriciaTree.get())
		{}
	};

	/// View on top of the metadata cache.
	class MetadataCacheView : public ReadOnlyViewSupplier<BasicMetadataCacheView> {
	public:
		/// Creates a view around \a metadataSets.
		explicit MetadataCacheView(const MetadataCacheTypes::BaseSets& metadataSets) : ReadOnlyViewSupplier(metadataSets)
		{}
	};
}}
