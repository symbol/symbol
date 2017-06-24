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
	DEFINE_MULTISIG_NOTIFICATION(Modify_Settings, 0x1001, All);

#undef DEFINE_MULTISIG_NOTIFICATION

	// endregion

	/// Notification of a multisig cosigners modification.
	struct ModifyMultisigCosignersNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Multisig_Modify_Cosigners_Notification;

	public:
		/// Creates a notification around \a signer, \a modificationsCount and \a pModifications.
		explicit ModifyMultisigCosignersNotification(
				const Key& signer,
				uint8_t modificationsCount,
				const CosignatoryModification* pModifications)
				: Notification(Notification_Type, sizeof(ModifyMultisigCosignersNotification))
				, Signer(signer)
				, ModificationsCount(modificationsCount)
				, ModificationsPtr(pModifications)
		{}

	public:
		/// The signer.
		const Key& Signer;

		/// The number of modifications.
		uint8_t ModificationsCount;

		/// Const pointer to the first modification.
		const CosignatoryModification* ModificationsPtr;
	};

	/// Notification of a new cosigner.
	struct ModifyMultisigNewCosignerNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Multisig_Modify_New_Cosigner_Notification;

	public:
		/// Creates a notification around \a multisigAccountKey and \a cosignatoryKey.
		explicit ModifyMultisigNewCosignerNotification(const Key& multisigAccountKey, const Key& cosignatoryKey)
				: Notification(Notification_Type, sizeof(ModifyMultisigNewCosignerNotification))
				, MultisigAccountKey(multisigAccountKey)
				, CosignatoryKey(cosignatoryKey)
		{}

	public:
		/// The multisig account key.
		const Key& MultisigAccountKey;

		/// The new cosignatory key.
		const Key& CosignatoryKey;
	};

	/// Notification of a multisig settings modification.
	struct ModifyMultisigSettingsNotification : public Notification {
	public:
		/// The matching notification type.
		static constexpr auto Notification_Type = Multisig_Modify_Settings_Notification;

	public:
		/// Creates a notification around \a signer, \a minRemovalDelta and \a minApprovalDelta.
		explicit ModifyMultisigSettingsNotification(const Key& signer, int8_t minRemovalDelta, int8_t minApprovalDelta)
				: Notification(Notification_Type, sizeof(ModifyMultisigSettingsNotification))
				, Signer(signer)
				, MinRemovalDelta(minRemovalDelta)
				, MinApprovalDelta(minApprovalDelta)
		{}

	public:
		/// The signer.
		const Key& Signer;

		/// The relative change of cosigs needed to remove another cosig.
		int8_t MinRemovalDelta;

		/// The relative change of cosigs needed to approve a transaction.
		int8_t MinApprovalDelta;
	};
}}
