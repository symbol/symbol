#pragma once
#include "ArtifactInfoAttributes.h"
#include "NamespaceConstants.h"
#include "catapult/model/TrailingVariableDataLayout.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a namespace info.
	struct NamespaceInfo : public TrailingVariableDataLayout<NamespaceInfo, NamespaceId> {
		/// The namespace id.
		NamespaceId Id;

		/// The namespace attributes.
		ArtifactInfoAttributes Attributes;

		/// The number of child namespaces.
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
