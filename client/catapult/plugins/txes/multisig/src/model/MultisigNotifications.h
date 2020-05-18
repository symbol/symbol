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
		/// Creates a notification around \a signerPublicKey, \a publicKeyAdditionsCount, \a pPublicKeyAdditions,
		/// \a publicKeyDeletionsCount and \a pPublicKeyDeletions.
		MultisigCosignatoriesNotification(
				const Key& signerPublicKey,
				uint8_t publicKeyAdditionsCount,
				const Key* pPublicKeyAdditions,
				uint8_t publicKeyDeletionsCount,
				const Key* pPublicKeyDeletions)
				: Notification(Notification_Type, sizeof(MultisigCosignatoriesNotification))
				, SignerPublicKey(signerPublicKey)
				, PublicKeyAdditionsCount(publicKeyAdditionsCount)
				, PublicKeyAdditionsPtr(pPublicKeyAdditions)
				, PublicKeyDeletionsCount(publicKeyDeletionsCount)
				, PublicKeyDeletionsPtr(pPublicKeyDeletions)
		{}

	public:
		/// Signer public key.
		const Key& SignerPublicKey;

		/// Number of cosignatory public key additions.
		uint8_t PublicKeyAdditionsCount;

		/// Const pointer to the first public key to add as cosignatory.
		const Key* PublicKeyAdditionsPtr;

		/// Number of cosignatory public key deletions.
		uint8_t PublicKeyDeletionsCount;

		/// Const pointer to the first public key to remove as cosignatory.
		const Key* PublicKeyDeletionsPtr;
	};

	// endregion

	// region MultisigNewCosignatoryNotification

	/// Notification of a new cosignatory.
	struct MultisigNewCosignatoryNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_New_Cosignatory_Notification;

	public:
		/// Creates a notification around \a multisigPublicKey and \a cosignatoryPublicKey.
		MultisigNewCosignatoryNotification(const Key& multisigPublicKey, const Key& cosignatoryPublicKey)
				: Notification(Notification_Type, sizeof(MultisigNewCosignatoryNotification))
				, MultisigPublicKey(multisigPublicKey)
				, CosignatoryPublicKey(cosignatoryPublicKey)
		{}

	public:
		/// Multisig public key.
		const Key& MultisigPublicKey;

		/// New cosignatory public key.
		const Key& CosignatoryPublicKey;
	};

	// endregion

	// region MultisigSettingsNotification

	/// Notification of a multisig settings modification.
	struct MultisigSettingsNotification : public Notification {
	public:
		/// Matching notification type.
		static constexpr auto Notification_Type = Multisig_Settings_Notification;

	public:
		/// Creates a notification around \a multisigPublicKey, \a minRemovalDelta and \a minApprovalDelta.
		MultisigSettingsNotification(const Key& multisigPublicKey, int8_t minRemovalDelta, int8_t minApprovalDelta)
				: Notification(Notification_Type, sizeof(MultisigSettingsNotification))
				, MultisigPublicKey(multisigPublicKey)
				, MinRemovalDelta(minRemovalDelta)
				, MinApprovalDelta(minApprovalDelta)
		{}

	public:
		/// Multisig public key.
		const Key& MultisigPublicKey;

		/// Relative change of cosigs needed to remove another cosig.
		int8_t MinRemovalDelta;

		/// Relative change of cosigs needed to approve a transaction.
		int8_t MinApprovalDelta;
	};

	// endregion
}}
