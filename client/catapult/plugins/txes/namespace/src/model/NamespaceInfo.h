#pragma once
#include "ArtifactInfoAttributes.h"
#include "NamespaceConstants.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a namespace info.
	struct NamespaceInfo {
		/// Size of the namespace info.
		uint32_t Size;

		/// The namespace id.
		NamespaceId Id;

		/// The namespace attributes.
		ArtifactInfoAttributes Attributes;

		/// The number of child namespaces.
		uint16_t NumChildren;

	private:
		static const uint8_t* ToBytePointer(const NamespaceInfo& namespaceInfo) {
			return reinterpret_cast<const uint8_t*>(&namespaceInfo);
		}

		static uint8_t* ToBytePointer(NamespaceInfo& namespaceInfo) {
			return reinterpret_cast<uint8_t*>(&namespaceInfo);
		}

		template<typename T>
		static auto PayloadStart(T& namespaceInfo) {
			return namespaceInfo.Size != CalculateRealSize(namespaceInfo) ? nullptr : ToBytePointer(namespaceInfo) + sizeof(T);
		}

		static const NamespaceId* ToNamespaceIdPointer(const uint8_t* pData) {
			return reinterpret_cast<const NamespaceId*>(pData);
		}

		static NamespaceId* ToNamespaceIdPointer(uint8_t* pData) {
			return reinterpret_cast<NamespaceId*>(pData);
		}

	public:
		/// Returns a const pointer to the first child namespace id contained in this namespace info.
		const NamespaceId* ChildrenPtr() const {
			return NumChildren ? ToNamespaceIdPointer(PayloadStart(*this)) : nullptr;
		}

		/// Returns a pointer to the first child namespace id contained in this namespace info.
		NamespaceId* ChildrenPtr() {
			return NumChildren ? ToNamespaceIdPointer(PayloadStart(*this)) : nullptr;
		}

	private:
		static constexpr uint64_t CalculateRealSize(const NamespaceInfo& namespaceInfo) noexcept {
			return sizeof(NamespaceInfo) + namespaceInfo.NumChildren * sizeof(NamespaceId);
		}

		friend constexpr uint64_t CalculateRealSize(const NamespaceInfo& namespaceInfo) noexcept;
	};

#pragma pack(pop)

	/// Calculates the real size of \a namespaceInfo.
	constexpr uint64_t CalculateRealSize(const NamespaceInfo& namespaceInfo) noexcept {
		return NamespaceInfo::CalculateRealSize(namespaceInfo);
	}

	/// Checks the real size of \a namespaceInfo against its reported size and returns \c true if the sizes match.
	constexpr bool IsSizeValid(const NamespaceInfo& namespaceInfo) noexcept {
		return CalculateRealSize(namespaceInfo) == namespaceInfo.Size;
	}
}}
