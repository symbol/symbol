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
#include "plugins/txes/namespace/src/types.h"
#include <vector>

namespace catapult { namespace model {

	/// Metadata type.
	enum class MetadataType : uint8_t {
		/// Account metadata.
		Account,

		/// Mosaic metadata.
		Mosaic,

		/// Namespace metadata.
		Namespace
	};

	/// Partial metadata key shared by all types of metadata.
	template<typename TTargetAddress>
	struct PartialMetadataKeyT {
		/// Address of the metadata source (provider).
		Address SourceAddress;

		/// Address of the metadata target.
		TTargetAddress TargetAddress;

		/// Metadata key scoped to source, target and type.
		uint64_t ScopedMetadataKey;
	};

	/// Partial metadata key shared by all types of metadata.
	using PartialMetadataKey = PartialMetadataKeyT<Address>;

	/// Unresolved partial metadata key shared by all types of metadata.
	using UnresolvedPartialMetadataKey = PartialMetadataKeyT<UnresolvedAddress>;

	/// Metadata target.
	struct MetadataTarget {
		/// Target type.
		MetadataType Type;

		/// Raw target identifier.
		uint64_t Id;
	};
}}
