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
#include "src/model/AccountRestrictionTypes.h"
#include "src/state/AccountRestrictionDescriptor.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region account restriction notification types

/// Defines an account restriction notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) \
	DEFINE_NOTIFICATION_TYPE(CHANNEL, RestrictionAccount, DESCRIPTION, CODE)

	/// Account restriction type.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Type, 0x0001, Validator);

	/// Account address restriction modification.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Address_Modification, 0x0010, All);

	/// Account mosaic restriction modification.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Mosaic_Modification, 0x0011, All);

	/// Account operation restriction modification.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Operation_Modification, 0x0012, All);

	/// Account address restriction modifications.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Address_Modifications, 0x0020, Validator);

	/// Account mosaic restriction modifications.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Mosaic_Modifications, 0x0021, Validator);

	/// Account operation restriction modifications.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Operation_Modifications, 0x0022, Validator);

#undef DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION

	// endregion

	// region AccountRestrictionTypeNotification

	/// Notification of an account restriction type.
	struct AccountRestrictionTypeNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = RestrictionAccount_Type_Notification;

	public:
		/// Creates a notification around \a restrictionType.
		explicit AccountRestrictionTypeNotification(model::AccountRestrictionType restrictionType)
				: Notification(Notification_Type, sizeof(AccountRestrictionTypeNotification))
				, AccountRestrictionType(restrictionType)
		{}

	public:
		/// Account restriction type.
		model::AccountRestrictionType AccountRestrictionType;
	};

	// endregion

	// region ModifyAccountRestrictionValueNotification

	/// Notification of an account restriction value modification.
	template<typename TRestrictionValue, NotificationType RestrictionAccount_Notification_Type>
	struct ModifyAccountRestrictionValueNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = RestrictionAccount_Notification_Type;

	public:
		/// Creates a notification around \a key, \a restrictionType and \a modification.
		ModifyAccountRestrictionValueNotification(
				const Key& key,
				AccountRestrictionType restrictionType,
				const AccountRestrictionModification<TRestrictionValue>& modification)
				: Notification(Notification_Type, sizeof(ModifyAccountRestrictionValueNotification))
				, Key(key)
				, AccountRestrictionDescriptor(restrictionType)
				, Modification(modification)
		{}

	public:
		/// Account's public key.
		catapult::Key Key;

		/// Account restriction descriptor.
		state::AccountRestrictionDescriptor AccountRestrictionDescriptor;

		/// Account restriction modification.
		/// \note TRestrictionValue is the resolved value.
		AccountRestrictionModification<TRestrictionValue> Modification;
	};

	using ModifyAccountAddressRestrictionValueNotification =
		ModifyAccountRestrictionValueNotification<UnresolvedAddress, RestrictionAccount_Address_Modification_Notification>;
	using ModifyAccountMosaicRestrictionValueNotification =
		ModifyAccountRestrictionValueNotification<UnresolvedMosaicId, RestrictionAccount_Mosaic_Modification_Notification>;
	using ModifyAccountOperationRestrictionValueNotification =
		ModifyAccountRestrictionValueNotification<EntityType, RestrictionAccount_Operation_Modification_Notification>;

	// endregion

	// region ModifyAccountRestrictionNotification

	/// Notification of an account restriction modification.
	template<typename TRestrictionValue, NotificationType RestrictionAccount_Notification_Type>
	struct ModifyAccountRestrictionNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = RestrictionAccount_Notification_Type;

	public:
		/// Creates a notification around \a key, \a restrictionType, \a modificationsCount and \a pModifications.
		ModifyAccountRestrictionNotification(
				const Key& key,
				AccountRestrictionType restrictionType,
				uint8_t modificationsCount,
				const AccountRestrictionModification<TRestrictionValue>* pModifications)
				: Notification(Notification_Type, sizeof(ModifyAccountRestrictionNotification))
				, Key(key)
				, AccountRestrictionDescriptor(restrictionType)
				, ModificationsCount(modificationsCount)
				, ModificationsPtr(pModifications)
		{}

	public:
		/// Account's public key.
		catapult::Key Key;

		/// Account restriction descriptor.
		state::AccountRestrictionDescriptor AccountRestrictionDescriptor;

		/// Number of modifications.
		uint8_t ModificationsCount;

		/// Const pointer to the first modification.
		const AccountRestrictionModification<TRestrictionValue>* ModificationsPtr;
	};

	using ModifyAccountAddressRestrictionNotification = ModifyAccountRestrictionNotification<
		UnresolvedAddress,
		RestrictionAccount_Address_Modifications_Notification>;
	using ModifyAccountMosaicRestrictionNotification = ModifyAccountRestrictionNotification<
		UnresolvedMosaicId,
		RestrictionAccount_Mosaic_Modifications_Notification>;
	using ModifyAccountOperationRestrictionNotification = ModifyAccountRestrictionNotification<
		EntityType,
		RestrictionAccount_Operation_Modifications_Notification>;

	// endregion
}}
