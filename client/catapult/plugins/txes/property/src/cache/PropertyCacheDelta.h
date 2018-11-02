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
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/model/NetworkInfo.h"

namespace catapult { namespace cache {

	/// Mixins used by the property cache delta.
	using PropertyCacheDeltaMixins = PatriciaTreeCacheMixins<PropertyCacheTypes::PrimaryTypes::BaseSetDeltaType, PropertyCacheDescriptor>;

	/// Basic delta on top of the property cache.
	class BasicPropertyCacheDelta
			: public utils::MoveOnly
			, public PropertyCacheDeltaMixins::Size
			, public PropertyCacheDeltaMixins::Contains
			, public PropertyCacheDeltaMixins::ConstAccessor
			, public PropertyCacheDeltaMixins::MutableAccessor
			, public PropertyCacheDeltaMixins::PatriciaTreeDelta
			, public PropertyCacheDeltaMixins::BasicInsertRemove
			, public PropertyCacheDeltaMixins::DeltaElements {
	public:
		using ReadOnlyView = PropertyCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a propertySets and \a networkIdentifier.
		explicit BasicPropertyCacheDelta(
				const PropertyCacheTypes::BaseSetDeltaPointers& propertySets,
				model::NetworkIdentifier networkIdentifier)
				: PropertyCacheDeltaMixins::Size(*propertySets.pPrimary)
				, PropertyCacheDeltaMixins::Contains(*propertySets.pPrimary)
				, PropertyCacheDeltaMixins::ConstAccessor(*propertySets.pPrimary)
				, PropertyCacheDeltaMixins::MutableAccessor(*propertySets.pPrimary)
				, PropertyCacheDeltaMixins::PatriciaTreeDelta(*propertySets.pPrimary, propertySets.pPatriciaTree)
				, PropertyCacheDeltaMixins::BasicInsertRemove(*propertySets.pPrimary)
				, PropertyCacheDeltaMixins::DeltaElements(*propertySets.pPrimary)
				, m_pPropertyEntries(propertySets.pPrimary)
				, m_networkIdentifier(networkIdentifier)
		{}

	public:
		using PropertyCacheDeltaMixins::ConstAccessor::find;
		using PropertyCacheDeltaMixins::MutableAccessor::find;

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const {
			return m_networkIdentifier;
		}

	private:
		PropertyCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pPropertyEntries;
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// Delta on top of the property cache.
	class PropertyCacheDelta : public ReadOnlyViewSupplier<BasicPropertyCacheDelta> {
	public:
		/// Creates a delta around \a propertySets and \a networkIdentifier.
		explicit PropertyCacheDelta(
				const PropertyCacheTypes::BaseSetDeltaPointers& propertySets,
				model::NetworkIdentifier networkIdentifier)
				: ReadOnlyViewSupplier(propertySets, networkIdentifier)
		{}
	};
}}
