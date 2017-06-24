#pragma once
#include "catapult/crypto/Hashes.h"
#include "catapult/model/Notifications.h"
#include "catapult/preprocessor.h"

namespace catapult { namespace test {

	/// Creates a new notification with \a type.
	CATAPULT_INLINE
	model::Notification CreateNotification(model::NotificationType type) {
		return model::Notification(type, sizeof(model::Notification));
	}

	/// Creates a placeholder block notification.
	CATAPULT_INLINE
	model::BlockNotification CreateBlockNotification() {
		// notice that notification Signer will be garbage after this returns
		return model::BlockNotification(Key(), Timestamp(), Difficulty());
	}

	/// Creates a block notification around \a signer.
	CATAPULT_INLINE
	model::BlockNotification CreateBlockNotification(const Key& signer) {
		return model::BlockNotification(signer, Timestamp(), Difficulty());
	}

	/// Casts \a notification to a derived notification type.
	template<typename TNotification>
	const TNotification& CastToDerivedNotification(const model::Notification& notification) {
		if (sizeof(TNotification) != notification.Size)
			CATAPULT_THROW_INVALID_ARGUMENT_1("notification has incorrect size", utils::to_underlying_type(notification.Type));

		return static_cast<const TNotification&>(notification);
	}

	/// Calculates the hash of \a notification.
	CATAPULT_INLINE
	Hash256 CalculateNotificationHash(const model::Notification& notification) {
		Hash256 notificationHash;
		crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(&notification), notification.Size }, notificationHash);
		return notificationHash;
	}
}}
