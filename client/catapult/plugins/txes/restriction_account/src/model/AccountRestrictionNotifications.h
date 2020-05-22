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
#include "AccountRestrictionFlags.h"
#include "AccountRestrictionModification.h"
#include "src/state/AccountRestrictionDescriptor.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region account restriction notification types

/// Defines an account restriction notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) \
	DEFINE_NOTIFICATION_TYPE(CHANNEL, RestrictionAccount, DESCRIPTION, CODE)

	/// Account restriction modification.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Modification, 0x0001, Validator);

	/// Account address restriction modification.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Address_Modification, 0x0002, All);

	/// Account mosaic restriction modification.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Mosaic_Modification, 0x0003, All);

	/// Account operation restriction modification.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Operation_Modification, 0x0004, All);

	/// Account address restriction modifications.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Address_Modifications, 0x0005, Validator);

	/// Account mosaic restriction modifications.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Mosaic_Modifications, 0x0006, Validator);

	/// Account operation restriction modifications.
	DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION(Operation_Modifications, 0x0007, Validator);

#undef DEFINE_ACCOUNT_RESTRICTION_NOTIFICATION

	// endregion

	// region AccountRestrictionModificationNotification

	/// Notification of an account restriction modification.
	struct AccountRestrictionModificationNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = RestrictionAccount_Modification_Notification;

	public:
		/// Creates a notification around \a restrictionFlags, \a restrictionAdditionsCount and \a restrictionDeletionsCount.
		AccountRestrictionModificationNotification(
				model::AccountRestrictionFlags restrictionFlags,
				uint8_t restrictionAdditionsCount,
				uint8_t restrictionDeletionsCount)
				: Notification(Notification_Type, sizeof(AccountRestrictionModificationNotification))
				, RestrictionFlags(restrictionFlags)
				, RestrictionAdditionsCount(restrictionAdditionsCount)
				, RestrictionDeletionsCount(restrictionDeletionsCount)
		{}

	public:
		/// Account restriction flags.
		AccountRestrictionFlags RestrictionFlags;

		/// Number of account restriction additions.
		uint8_t RestrictionAdditionsCount;

		/// Number of account restriction deletions.
		uint8_t RestrictionDeletionsCount;
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
		/// Creates a notification around \a address, \a restrictionFlags, \a restrictionValue and \a action.
		ModifyAccountRestrictionValueNotification(
				const Address& address,
				AccountRestrictionFlags restrictionFlags,
				const TRestrictionValue& restrictionValue,
				AccountRestrictionModificationAction action)
				: Notification(Notification_Type, sizeof(ModifyAccountRestrictionValueNotification))
				, Address(address)
				, AccountRestrictionDescriptor(restrictionFlags)
				, RestrictionValue(restrictionValue)
				, Action(action)
		{}

	public:
		/// Account's address.
		catapult::Address Address;

		/// Account restriction descriptor.
		state::AccountRestrictionDescriptor AccountRestrictionDescriptor;

		/// Account restriction value.
		const TRestrictionValue& RestrictionValue;

		/// Account restriction modification action.
		AccountRestrictionModificationAction Action;
	};

	using ModifyAccountAddressRestrictionValueNotification =
		ModifyAccountRestrictionValueNotification<UnresolvedAddress, RestrictionAccount_Address_Modification_Notification>;
	using ModifyAccountMosaicRestrictionValueNotification =
		ModifyAccountRestrictionValueNotification<UnresolvedMosaicId, RestrictionAccount_Mosaic_Modification_Notification>;
	using ModifyAccountOperationRestrictionValueNotification =
		ModifyAccountRestrictionValueNotification<EntityType, RestrictionAccount_Operation_Modification_Notification>;

	// endregion

	// region ModifyAccountRestrictionsNotification

	/// Notification of an account restrictions modification.
	template<typename TRestrictionValue, NotificationType RestrictionAccount_Notification_Type>
	struct ModifyAccountRestrictionsNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = RestrictionAccount_Notification_Type;

	public:
		/// Creates a notification around \a address, \a restrictionFlags, \a restrictionAdditionsCount, \a pRestrictionAdditions,
		/// \a restrictionDeletionsCount and \a pRestrictionDeletions.
		ModifyAccountRestrictionsNotification(
				const Address& address,
				AccountRestrictionFlags restrictionFlags,
				uint8_t restrictionAdditionsCount,
				const TRestrictionValue* pRestrictionAdditions,
				uint8_t restrictionDeletionsCount,
				const TRestrictionValue* pRestrictionDeletions)
				: Notification(Notification_Type, sizeof(ModifyAccountRestrictionsNotification))
				, Address(address)
				, AccountRestrictionDescriptor(restrictionFlags)
				, RestrictionAdditionsCount(restrictionAdditionsCount)
				, RestrictionAdditionsPtr(pRestrictionAdditions)
				, RestrictionDeletionsCount(restrictionDeletionsCount)
				, RestrictionDeletionsPtr(pRestrictionDeletions)
		{}

	public:
		/// Account's address.
		catapult::Address Address;

		/// Account restriction descriptor.
		state::AccountRestrictionDescriptor AccountRestrictionDescriptor;

		/// Number of account restriction additions.
		uint8_t RestrictionAdditionsCount;

		/// Const pointer to the first account restriction to add as constraint.
		const TRestrictionValue* RestrictionAdditionsPtr;

		/// Number of account restriction deletions.
		uint8_t RestrictionDeletionsCount;

		/// Const pointer to the first account restriction to remove as constraint.
		const TRestrictionValue* RestrictionDeletionsPtr;
	};

	using ModifyAccountAddressRestrictionsNotification = ModifyAccountRestrictionsNotification<
		UnresolvedAddress,
		RestrictionAccount_Address_Modifications_Notification>;
	using ModifyAccountMosaicRestrictionsNotification = ModifyAccountRestrictionsNotification<
		UnresolvedMosaicId,
		RestrictionAccount_Mosaic_Modifications_Notification>;
	using ModifyAccountOperationRestrictionsNotification = ModifyAccountRestrictionsNotification<
		EntityType,
		RestrictionAccount_Operation_Modifications_Notification>;

	// endregion
}}
