#pragma once
#include "EntityBody.h"
#include "SizePrefixedEntity.h"
#include <iosfwd>

namespace catapult {
	namespace model {
		class NotificationSubscriber;
		class TransactionRegistry;
	}
}

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for an embedded transaction (non-verifiable).
	struct EmbeddedTransaction : public EntityBody<SizePrefixedEntity> {
	};

#pragma pack(pop)

	/// Insertion operator for outputting \a transaction to \a out.
	std::ostream& operator<<(std::ostream& out, const EmbeddedTransaction& transaction);

	/// Checks the real size of \a transaction against its reported size and returns \c true if the sizes match.
	/// \a registry contains all known transaction types.
	bool IsSizeValid(const EmbeddedTransaction& transaction, const TransactionRegistry& registry);

	/// Sends all notifications from \a transaction to \a sub.
	void PublishNotifications(const EmbeddedTransaction& transaction, NotificationSubscriber& sub);
}}
