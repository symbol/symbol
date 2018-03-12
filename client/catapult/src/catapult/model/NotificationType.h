#pragma once
#include "FacilityCode.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace model {

	/// Notification channel.
	enum class NotificationChannel : uint8_t {
		/// Publish notification on no channels.
		None = 0x00,
		/// Publish notification on validator channel.
		Validator = 0x01,
		/// Publish notification on observer channel.
		Observer = 0x02,
		/// Publish notification on all channels.
		All = 0xFF
	};

	/// Enumeration of all possible notification types.
	enum class NotificationType : uint32_t {
	};

	/// Makes a notification type given \a channel, \a facility and \a code.
	constexpr NotificationType MakeNotificationType(NotificationChannel channel, FacilityCode facility, uint16_t code) {
		return static_cast<NotificationType>(
				static_cast<uint32_t>(channel) << 24 | //  01..08: channel
				static_cast<uint32_t>(facility) << 16 | // 09..16: facility
				code); //                                  16..32: code
	}

/// Defines a notification type given \a CHANNEL, \a FACILITY, \a DESCRIPTION and \a CODE.
#define DEFINE_NOTIFICATION_TYPE(CHANNEL, FACILITY, DESCRIPTION, CODE) \
	constexpr auto FACILITY##_##DESCRIPTION##_Notification = model::MakeNotificationType( \
			(model::NotificationChannel::CHANNEL), \
			(model::FacilityCode::FACILITY), \
			CODE)

	/// Checks if \a type has \a channel set.
	constexpr bool IsSet(NotificationType type, NotificationChannel channel) {
		return utils::to_underlying_type(channel) == (utils::to_underlying_type(channel) & (utils::to_underlying_type(type) >> 24));
	}

	/// Gets the notification channel set in \a type.
	constexpr NotificationChannel GetNotificationChannel(NotificationType type) {
		return static_cast<NotificationChannel>(utils::to_underlying_type(type) >> 24);
	}

	/// Sets the notification channel in \a type to \a channel.
	CPP14_CONSTEXPR void SetNotificationChannel(NotificationType& type, NotificationChannel channel) {
		type = static_cast<NotificationType>(
				static_cast<uint32_t>(utils::to_underlying_type(channel) << 24u) |
				(0x00FFFFFFu & utils::to_underlying_type(type)));
	}

	/// Returns true if \a lhs and \a rhs have the same source (facility and code).
	constexpr bool AreEqualExcludingChannel(NotificationType lhs, NotificationType rhs) {
		return (0x00FFFFFFu & utils::to_underlying_type(lhs)) == (0x00FFFFFFu & utils::to_underlying_type(rhs));
	}

	// region core notification types

/// Defines a core notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_CORE_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Core, DESCRIPTION, CODE)

	/// Account was used with specified address.
	DEFINE_CORE_NOTIFICATION(Register_Account_Address, 0x0001, All);

	/// Account was used with specified public key.
	DEFINE_CORE_NOTIFICATION(Register_Account_Public_Key, 0x0002, Observer);

	/// Mosaic was transferred between two accounts.
	DEFINE_CORE_NOTIFICATION(Balance_Transfer, 0x0003, All);

	/// Entity was received.
	DEFINE_CORE_NOTIFICATION(Entity, 0x0004, Validator);

	/// Block was received.
	DEFINE_CORE_NOTIFICATION(Block, 0x0005, All);

	/// Transaction was received.
	DEFINE_CORE_NOTIFICATION(Transaction, 0x0006, All);

	/// Signature was received.
	DEFINE_CORE_NOTIFICATION(Signature, 0x0007, Validator);

	/// Account is required to have minimum balance.
	DEFINE_CORE_NOTIFICATION(Balance_Reserve, 0x0008, Validator);

#undef DEFINE_CORE_NOTIFICATION

	// endregion
}}
