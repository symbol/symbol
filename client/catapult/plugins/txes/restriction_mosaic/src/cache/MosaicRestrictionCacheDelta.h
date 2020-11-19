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
#include "MosaicRestrictionBaseSets.h"
#include "ReadOnlyMosaicRestrictionCache.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/model/NetworkIdentifier.h"

namespace catapult { namespace cache {

	/// Mixins used by the mosaic restriction cache delta.
	using MosaicRestrictionCacheDeltaMixins =
		PatriciaTreeCacheMixins<MosaicRestrictionCacheTypes::PrimaryTypes::BaseSetDeltaType, MosaicRestrictionCacheDescriptor>;

	/// Basic delta on top of the mosaic restriction cache.
	class BasicMosaicRestrictionCacheDelta
			: public utils::MoveOnly
			, public MosaicRestrictionCacheDeltaMixins::Size
			, public MosaicRestrictionCacheDeltaMixins::Contains
			, public MosaicRestrictionCacheDeltaMixins::ConstAccessor
			, public MosaicRestrictionCacheDeltaMixins::MutableAccessor
			, public MosaicRestrictionCacheDeltaMixins::PatriciaTreeDelta
			, public MosaicRestrictionCacheDeltaMixins::BasicInsertRemove
			, public MosaicRestrictionCacheDeltaMixins::DeltaElements {
	public:
		using ReadOnlyView = MosaicRestrictionCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a restrictionSets and \a networkIdentifier.
		BasicMosaicRestrictionCacheDelta(
				const MosaicRestrictionCacheTypes::BaseSetDeltaPointers& restrictionSets,
				model::NetworkIdentifier networkIdentifier)
				: MosaicRestrictionCacheDeltaMixins::Size(*restrictionSets.pPrimary)
				, MosaicRestrictionCacheDeltaMixins::Contains(*restrictionSets.pPrimary)
				, MosaicRestrictionCacheDeltaMixins::ConstAccessor(*restrictionSets.pPrimary)
				, MosaicRestrictionCacheDeltaMixins::MutableAccessor(*restrictionSets.pPrimary)
				, MosaicRestrictionCacheDeltaMixins::PatriciaTreeDelta(*restrictionSets.pPrimary, restrictionSets.pPatriciaTree)
				, MosaicRestrictionCacheDeltaMixins::BasicInsertRemove(*restrictionSets.pPrimary)
				, MosaicRestrictionCacheDeltaMixins::DeltaElements(*restrictionSets.pPrimary)
				, m_pMosaicRestrictionEntries(restrictionSets.pPrimary)
				, m_networkIdentifier(networkIdentifier)
		{}

	public:
		using MosaicRestrictionCacheDeltaMixins::ConstAccessor::find;
		using MosaicRestrictionCacheDeltaMixins::MutableAccessor::find;

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const {
			return m_networkIdentifier;
		}

	private:
		MosaicRestrictionCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pMosaicRestrictionEntries;
		model::NetworkIdentifier m_networkIdentifier;
	};

	/// Delta on top of the mosaic restriction cache.
	class MosaicRestrictionCacheDelta : public ReadOnlyViewSupplier<BasicMosaicRestrictionCacheDelta> {
	public:
		/// Creates a delta around \a restrictionSets and \a networkIdentifier.
		MosaicRestrictionCacheDelta(
				const MosaicRestrictionCacheTypes::BaseSetDeltaPointers& restrictionSets,
				model::NetworkIdentifier networkIdentifier)
				: ReadOnlyViewSupplier(restrictionSets, networkIdentifier)
		{}
	};
}}
