/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
	enum class NotificationType : uint32_t {};

	/// Makes a notification type given \a channel, \a facility and \a code.
	constexpr NotificationType MakeNotificationType(NotificationChannel channel, FacilityCode facility, uint16_t code) {
		return static_cast<NotificationType>(
				static_cast<uint32_t>(channel) << 24 //    01..08: channel
				| static_cast<uint32_t>(facility) << 16 // 09..16: facility
				| code); //                                16..32: code
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
	constexpr void SetNotificationChannel(NotificationType& type, NotificationChannel channel) {
		type = static_cast<NotificationType>(
				static_cast<uint32_t>(utils::to_underlying_type(channel) << 24)
				| (0x00FFFFFFu & utils::to_underlying_type(type)));
	}

	/// Returns \c true if \a lhs and \a rhs have the same source (facility and code).
	constexpr bool AreEqualExcludingChannel(NotificationType lhs, NotificationType rhs) {
		return (0x00FFFFFFu & utils::to_underlying_type(lhs)) == (0x00FFFFFFu & utils::to_underlying_type(rhs));
	}

	// region core notification types

/// Defines a core notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_CORE_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Core, DESCRIPTION, CODE)

	/// Account was used with specified address.
	DEFINE_CORE_NOTIFICATION(Register_Account_Address, 0x0001, All);

	/// Account was used with specified public key.
	DEFINE_CORE_NOTIFICATION(Register_Account_Public_Key, 0x0002, All);

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

	/// Mosaic was debited from account.
	DEFINE_CORE_NOTIFICATION(Balance_Debit, 0x0008, All);

	/// Source address interacts with destination addresses.
	DEFINE_CORE_NOTIFICATION(Address_Interaction, 0x0009, Validator);

	/// Mosaic is required.
	DEFINE_CORE_NOTIFICATION(Mosaic_Required, 0x000A, Validator);

	/// Source has changed.
	DEFINE_CORE_NOTIFICATION(Source_Change, 0x000B, Observer);

	/// Transaction fee was received.
	DEFINE_CORE_NOTIFICATION(Transaction_Fee, 0x000C, All);

	/// Transaction deadline was received.
	DEFINE_CORE_NOTIFICATION(Transaction_Deadline, 0x000D, Validator);

	/// Internal padding was identified.
	DEFINE_CORE_NOTIFICATION(Internal_Padding, 0x000E, Validator);

	/// Voting key was linked/unlinked.
	DEFINE_CORE_NOTIFICATION(Voting_Key_Link, 0x000F, All);

	/// Vrf key was linked/unlinked.
	DEFINE_CORE_NOTIFICATION(Vrf_Key_Link, 0x0010, All);

	/// Key link action was received.
	DEFINE_CORE_NOTIFICATION(Key_Link_Action, 0x0011, Validator);

	/// Importance block was received.
	/// \note This notification type cannot support observation without more substantial changes.
	/// CoreSystem observers assume that no notifications raised after Block can trigger state changes.
	/// ImportanceBlockValidator relies on Block_Importance being raised after Block for the same reason.
	DEFINE_CORE_NOTIFICATION(Block_Importance, 0x0012, Validator);

	/// Block was received (type information only).
	DEFINE_CORE_NOTIFICATION(Block_Type, 0x0013, Validator);

#undef DEFINE_CORE_NOTIFICATION

	// endregion
}}
