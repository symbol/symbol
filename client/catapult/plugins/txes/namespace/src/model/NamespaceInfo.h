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
#include "ArtifactInfoAttributes.h"
#include "NamespaceConstants.h"
#include "catapult/model/TrailingVariableDataLayout.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a namespace info.
	struct NamespaceInfo : public TrailingVariableDataLayout<NamespaceInfo, NamespaceId> {
		/// Namespace id.
		NamespaceId Id;

		/// Namespace attributes.
		ArtifactInfoAttributes Attributes;

		/// Number of child namespaces.
		uint16_t ChildCount;

	public:
		/// Returns a const pointer to the first child namespace id contained in this namespace info.
		const NamespaceId* ChildrenPtr() const {
			return ChildCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

		/// Returns a pointer to the first child namespace id contained in this namespace info.
		NamespaceId* ChildrenPtr() {
			return ChildCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

	public:
		/// Calculates the real size of \a namespaceInfo.
		static constexpr uint64_t CalculateRealSize(const NamespaceInfo& namespaceInfo) noexcept {
			return sizeof(NamespaceInfo) + namespaceInfo.ChildCount * sizeof(NamespaceId);
		}
	};

#pragma pack(pop)
}}
