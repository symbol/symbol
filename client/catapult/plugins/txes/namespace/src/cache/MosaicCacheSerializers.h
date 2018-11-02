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
#include "MosaicCacheTypes.h"
#include "src/state/MosaicHistorySerializer.h"
#include "catapult/cache/CacheSerializerAdapter.h"
#include "catapult/cache/IdentifierGroupSerializer.h"

namespace catapult { namespace cache {

	/// Primary serializer for mosaic cache.
	struct MosaicHistoryPrimarySerializer : public CacheSerializerAdapter<state::MosaicHistorySerializer, MosaicCacheDescriptor>
	{};

	/// Primary serializer for mosaic cache for patricia tree hashes.
	/// \note This serializer excludes historical entries.
	struct MosaicHistoryPatriciaTreeSerializer
			: public CacheSerializerAdapter<state::MosaicHistoryNonHistoricalSerializer, MosaicCacheDescriptor>
	{};

	/// Serializer for namespace grouping subcache.
	struct NamespaceGroupingSerializer : public IdentifierGroupSerializer<MosaicCacheTypes::NamespaceGroupingTypesDescriptor>
	{};

	/// Serializer for mosaic cache height grouped elements.
	struct MosaicHeightGroupingSerializer : public IdentifierGroupSerializer<MosaicCacheTypes::HeightGroupingTypesDescriptor>
	{};
}}
