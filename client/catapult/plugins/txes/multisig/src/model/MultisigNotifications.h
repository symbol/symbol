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
#include "ModifyMultisigAccountTransaction.h"
#include "catapult/model/Notifications.h"

namespace catapult { namespace model {

	// region multisig notification types

/// Defines a multisig notification type with \a DESCRIPTION, \a CODE and \a CHANNEL.
#define DEFINE_MULTISIG_NOTIFICATION(DESCRIPTION, CODE, CHANNEL) DEFINE_NOTIFICATION_TYPE(CHANNEL, Multisig, DESCRIPTION, CODE)

	/// Multisig account cosigners were modified.
	DEFINE_MULTISIG_NOTIFICATION(Modify_Cosigners, 0x0001, All);

	/// A cosigner was added to a multisig account.
	DEFINE_MULTISIG_NOTIFICATION(Modify_New_Cosigner, 0x0002, Validator);

	/// Multisig account settings were modified.
	DEFINE_MULTISIG_NOTIFICATION(Modify_Settings, 0x0003, All);

#undef DEFINE_MULTISIG_NOTIFICATION

	// endregion

	// region ModifyMultisigCosignersNotification

	/// Notification of a multisig cosigners modification.
	struct ModifyMultisigCosignersNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_Modify_Cosigners_Notification;

	public:
		/// Creates a notification around \a signer, \a modificationsCount and \a pModifications.
		ModifyMultisigCosignersNotification(const Key& signer, uint8_t modificationsCount, const CosignatoryModification* pModifications)
				: Notification(Notification_Type, sizeof(ModifyMultisigCosignersNotification))
				, Signer(signer)
				, ModificationsCount(modificationsCount)
				, ModificationsPtr(pModifications)
		{}

	public:
		/// Signer.
		const Key& Signer;

		/// Number of modifications.
		uint8_t ModificationsCount;

		/// Const pointer to the first modification.
		const CosignatoryModification* ModificationsPtr;
	};

	// endregion

	// region ModifyMultisigNewCosignerNotification

	/// Notification of a new cosigner.
	struct ModifyMultisigNewCosignerNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_Modify_New_Cosigner_Notification;

	public:
		/// Creates a notification around \a multisigAccountKey and \a cosignatoryKey.
		ModifyMultisigNewCosignerNotification(const Key& multisigAccountKey, const Key& cosignatoryKey)
				: Notification(Notification_Type, sizeof(ModifyMultisigNewCosignerNotification))
				, MultisigAccountKey(multisigAccountKey)
				, CosignatoryKey(cosignatoryKey)
		{}

	public:
		/// Multisig account key.
		const Key& MultisigAccountKey;

		/// New cosignatory key.
		const Key& CosignatoryKey;
	};

	// endregion

	// region ModifyMultisigSettingsNotification

	/// Notification of a multisig settings modification.
	struct ModifyMultisigSettingsNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_Modify_Settings_Notification;

	public:
		/// Creates a notification around \a signer, \a minRemovalDelta and \a minApprovalDelta.
		ModifyMultisigSettingsNotification(const Key& signer, int8_t minRemovalDelta, int8_t minApprovalDelta)
				: Notification(Notification_Type, sizeof(ModifyMultisigSettingsNotification))
				, Signer(signer)
				, MinRemovalDelta(minRemovalDelta)
				, MinApprovalDelta(minApprovalDelta)
		{}

	public:
		/// Signer.
		const Key& Signer;

		/// Relative change of cosigs needed to remove another cosig.
		int8_t MinRemovalDelta;

		/// Relative change of cosigs needed to approve a transaction.
		int8_t MinApprovalDelta;
	};

	// endregion
}}
