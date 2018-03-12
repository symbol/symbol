#pragma once
#include "EntityType.h"
#include "NetworkInfo.h"
#include "catapult/utils/Casting.h"
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an entity body.
	template<typename THeader>
	struct EntityBody : public THeader {
	public:
		/// The entity signer's public key.
		Key Signer;

		/// The entity version.
		uint16_t Version;

		/// The entity type.
		EntityType Type;

		/// Returns network of an entity, as defined in NetworkInfoTraits.
		NetworkIdentifier Network() const {
			return static_cast<NetworkIdentifier>(Version >> 8);
		}

		/// Returns version of an entity.
		uint8_t EntityVersion() const {
			return static_cast<uint8_t>(Version & 0xFF);
		}
	};

#pragma pack(pop)

	/// Creates a version field out of given entity \a version and \a networkIdentifier.
	constexpr uint16_t MakeVersion(NetworkIdentifier networkIdentifier, uint8_t version) noexcept {
		return static_cast<uint16_t>(utils::to_underlying_type(networkIdentifier) << 8 | version);
	}
}}
