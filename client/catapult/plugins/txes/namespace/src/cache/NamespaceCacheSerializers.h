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
#include "NamespaceCacheTypes.h"
#include "src/state/RootNamespaceHistorySerializer.h"
#include "catapult/cache/CacheSerializerAdapter.h"
#include "catapult/cache/IdentifierGroupSerializer.h"

namespace catapult { namespace cache {

	/// Primary serializer for namespace cache.
	struct RootNamespaceHistoryPrimarySerializer
			: public CacheSerializerAdapter<state::RootNamespaceHistorySerializer, NamespaceCacheDescriptor>
	{};

	/// Primary serializer for namespace cache for patricia tree hashes.
	/// \note This serializer excludes historical namespaces.
	struct RootNamespaceHistoryPatriciaTreeSerializer
			: public CacheSerializerAdapter<state::RootNamespaceHistoryNonHistoricalSerializer, NamespaceCacheDescriptor>
	{};

	/// Serializer for namespace flat map sub cache.
	struct NamespaceFlatMapTypesSerializer {
	private:
		using KeyType = NamespaceId;
		using ValueType = state::Namespace;

	public:
		/// Serialized state version.
		static constexpr uint16_t State_Version = 1;

	public:
		/// Serializes \a element to string.
		static std::string SerializeValue(const ValueType& value);

		/// Deserializes value from \a buffer.
		static ValueType DeserializeValue(const RawBuffer& buffer);
	};

	/// Serializer for namespace cache height grouped elements.
	struct NamespaceHeightGroupingSerializer : public IdentifierGroupSerializer<NamespaceCacheTypes::HeightGroupingTypesDescriptor> {};
}}
