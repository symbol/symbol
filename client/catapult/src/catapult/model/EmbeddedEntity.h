#pragma once
#include "EntityType.h"
#include "NetworkInfo.h"
#include "SizePrefixedEntity.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/types.h"
#include <iosfwd>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace model {
		class NotificationSubscriber;
		class TransactionRegistry;
	}
}

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
		uint8_t Network() const {
			return static_cast<uint8_t>(Version >> 8);
		}

		/// Returns version of an entity.
		uint8_t EntityVersion() const {
			return static_cast<uint8_t>(Version & 0xFF);
		}
	};

	/// Binary layout for an embedded entity (non-verifiable).
	struct EmbeddedEntity : public EntityBody<SizePrefixedEntity> {
	};

#pragma pack(pop)

	/// Insertion operator for outputting \a entity to \a out.
	std::ostream& operator<<(std::ostream& out, const EmbeddedEntity& entity);

	/// Checks the real size of \a entity against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const EmbeddedEntity& entity, const TransactionRegistry& registry);

	/// Sends all notifications from \a entity to \a sub.
	void PublishNotifications(const EmbeddedEntity& entity, NotificationSubscriber& sub);

	/// Creates a version field out of given entity \a version and \a networkIdentifier.
	constexpr uint16_t MakeVersion(NetworkIdentifier networkIdentifier, uint8_t version) noexcept {
		return static_cast<uint16_t>(utils::to_underlying_type(networkIdentifier) << 8 | version);
	}
}}
