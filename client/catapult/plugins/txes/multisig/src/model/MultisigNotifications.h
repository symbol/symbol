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
#include "MultisigAccountModificationTransaction.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region multisig notification types

/// Defines a multisig notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_MULTISIG_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Multisig, DESCRIPTION, CODE)

	/// Multisig account cosignatories were modified.
	DEFINE_MULTISIG_NOTIFICATION(Cosignatories, 0x0001, All);

	/// Cosignatory was added to a multisig account.
	DEFINE_MULTISIG_NOTIFICATION(New_Cosignatory, 0x0002, Validator);

	/// Multisig account settings were modified.
	DEFINE_MULTISIG_NOTIFICATION(Settings, 0x0003, All);

#undef DEFINE_MULTISIG_NOTIFICATION

	// endregion

	// region MultisigCosignatoriesNotification

	/// Notification of a multisig cosignatories modification.
	struct MultisigCosignatoriesNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_Cosignatories_Notification;

	public:
		/// Creates a notification around \a multisig, \a addressAdditionsCount, \a pAddressAdditions,
		/// \a addressDeletionsCount and \a pAddressDeletions.
		MultisigCosignatoriesNotification(
				const Address& multisig,
				uint8_t addressAdditionsCount,
				const UnresolvedAddress* pAddressAdditions,
				uint8_t addressDeletionsCount,
				const UnresolvedAddress* pAddressDeletions)
				: Notification(Notification_Type, sizeof(MultisigCosignatoriesNotification))
				, Multisig(multisig)
				, AddressAdditionsCount(addressAdditionsCount)
				, AddressAdditionsPtr(pAddressAdditions)
				, AddressDeletionsCount(addressDeletionsCount)
				, AddressDeletionsPtr(pAddressDeletions)
		{}

	public:
		/// Multisig account.
		Address Multisig;

		/// Number of cosignatory address additions.
		uint8_t AddressAdditionsCount;

		/// Const pointer to the first address to add as cosignatory.
		const UnresolvedAddress* AddressAdditionsPtr;

		/// Number of cosignatory address deletions.
		uint8_t AddressDeletionsCount;

		/// Const pointer to the first address to remove as cosignatory.
		const UnresolvedAddress* AddressDeletionsPtr;
	};

	// endregion

	// region MultisigNewCosignatoryNotification

	/// Notification of a new cosignatory.
	struct MultisigNewCosignatoryNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_New_Cosignatory_Notification;

	public:
		/// Creates a notification around \a multisig and \a cosignatory.
		MultisigNewCosignatoryNotification(const Address& multisig, const UnresolvedAddress& cosignatory)
				: Notification(Notification_Type, sizeof(MultisigNewCosignatoryNotification))
				, Multisig(multisig)
				, Cosignatory(cosignatory)
		{}

	public:
		/// Multisig account.
		Address Multisig;

		/// New cosignatory account.
		UnresolvedAddress Cosignatory;
	};

	// endregion

	// region MultisigSettingsNotification

	/// Notification of a multisig settings modification.
	struct MultisigSettingsNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_Settings_Notification;

	public:
		/// Creates a notification around \a multisig, \a minRemovalDelta and \a minApprovalDelta.
		MultisigSettingsNotification(const Address& multisig, int8_t minRemovalDelta, int8_t minApprovalDelta)
				: Notification(Notification_Type, sizeof(MultisigSettingsNotification))
				, Multisig(multisig)
				, MinRemovalDelta(minRemovalDelta)
				, MinApprovalDelta(minApprovalDelta)
		{}

	public:
		/// Multisig account.
		Address Multisig;

		/// Relative change of cosigs needed to remove another cosig.
		int8_t MinRemovalDelta;

		/// Relative change of cosigs needed to approve a transaction.
		int8_t MinApprovalDelta;
	};

	// endregion
}}
