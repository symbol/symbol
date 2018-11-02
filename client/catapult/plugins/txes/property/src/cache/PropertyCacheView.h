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

#pragma once
#include "PropertyBaseSets.h"
#include "PropertyCacheSerializers.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the property cache view.
	using PropertyCacheViewMixins = PatriciaTreeCacheMixins<PropertyCacheTypes::PrimaryTypes::BaseSetType, PropertyCacheDescriptor>;

	/// Basic view on top of the property cache.
	class BasicPropertyCacheView
			: public utils::MoveOnly
			, public PropertyCacheViewMixins::Size
			, public PropertyCacheViewMixins::Contains
			, public PropertyCacheViewMixins::Iteration
			, public PropertyCacheViewMixins::ConstAccessor
			, public PropertyCacheViewMixins::PatriciaTreeView {
	public:
		using ReadOnlyView = PropertyCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a propertySets and \a networkIdentifier.
		explicit BasicPropertyCacheView(const PropertyCacheTypes::BaseSets& propertySets, model::NetworkIdentifier networkIdentifier)
				: PropertyCacheViewMixins::Size(propertySets.Primary)
				, PropertyCacheViewMixins::Contains(propertySets.Primary)
				, PropertyCacheViewMixins::Iteration(propertySets.Primary)
				, PropertyCacheViewMixins::ConstAccessor(propertySets.Primary)
				, PropertyCacheViewMixins::PatriciaTreeView(propertySets.PatriciaTree.get())
				, m_networkIdentifier(networkIdentifier)
		{}

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const {
			return m_networkIdentifier;
		}

	private:
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// View on top of the property cache.
	class PropertyCacheView : public ReadOnlyViewSupplier<BasicPropertyCacheView> {
	public:
		/// Creates a view around \a propertySets and \a networkIdentifier.
		explicit PropertyCacheView(const PropertyCacheTypes::BaseSets& propertySets, model::NetworkIdentifier networkIdentifier)
				: ReadOnlyViewSupplier(propertySets, networkIdentifier)
		{}
	};
}}
