#pragma once
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

	/// Possible notification facility codes.
	enum class NotificationFacilityCode : uint8_t {
		/// Aggregate facility code.
		Aggregate = 0x41,
		/// Core facility code.
		Core = 0x43,
		/// Mosaic facility code.
		Mosaic = 0x4D,
		/// Multisig facility code.
		Multisig = 0x55,
		/// Namespace facility code.
		Namespace = 0x4E,
		/// Transfer facility code.
		Transfer = 0x54
	};

	/// Enumeration of all possible notification types.
	enum class NotificationType : uint32_t {
	};

	/// Makes a notification type given \a channel, \a facility and \a code.
	constexpr NotificationType MakeNotificationType(NotificationChannel channel, NotificationFacilityCode facility, uint16_t code) {
		return static_cast<NotificationType>(
				static_cast<uint32_t>(channel) << 24 | // 01..08: flags
				static_cast<uint32_t>(facility) << 16 | // 09..16: facility
				code); // 16..32: code
	}

/// Defines a notification type given \a CHANNEL, \a FACILITY, \a DESCRIPTION and \a CODE.
#define DEFINE_NOTIFICATION_TYPE(CHANNEL, FACILITY, DESCRIPTION, CODE) \
	constexpr auto FACILITY##_##DESCRIPTION##_Notification = model::MakeNotificationType( \
			(model::NotificationChannel::CHANNEL), \
			(model::NotificationFacilityCode::FACILITY), \
			CODE)

	/// Checks if \a type has \a channel set.
	constexpr bool IsSet(NotificationType type, NotificationChannel channel) {
		return utils::to_underlying_type(channel) == (utils::to_underlying_type(channel) & (utils::to_underlying_type(type) >> 24));
	}

	// region core notification types

/// Defines a core notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_CORE_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Core, DESCRIPTION, CODE)

	/// Account was used with specified address.
	DEFINE_CORE_NOTIFICATION(Register_Account_Address, 0x0001, Observer);

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

#undef DEFINE_CORE_NOTIFICATION

	// endregion
}}
