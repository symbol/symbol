#pragma once
#include "ImportanceCalculator.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	// region VerifiableEntity

	/// Observes account address changes.
	DECLARE_OBSERVER(AccountAddress, model::AccountAddressNotification)();

	/// Observes account public key changes.
	DECLARE_OBSERVER(AccountPublicKey, model::AccountPublicKeyNotification)();

	// endregion

	// region Block

	/// Observes block notifications and triggers importance recalculations using either \a pCommitCalculator (for commits)
	/// or \a pRollbackCalculator (for rollbacks).
	DECLARE_OBSERVER(RecalculateImportances, model::BlockNotification)(
			std::unique_ptr<ImportanceCalculator>&& pCommitCalculator,
			std::unique_ptr<ImportanceCalculator>&& pRollbackCalculator);

	/// Observes block notifications and credits the harvester with transaction fees.
	DECLARE_OBSERVER(HarvestFee, model::BlockNotification)();

	/// Observes block difficulties.
	DECLARE_OBSERVER(BlockDifficulty, model::BlockNotification)();

	// endregion

	// region Transaction

	/// Observes balance changes triggered by balance transfer notifications.
	DECLARE_OBSERVER(Balance, model::BalanceTransferNotification)();

	// endregion
}}
