#pragma once
#include "ImportanceCalculator.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	// region VerifiableEntity

	/// Observes account address changes.
	NotificationObserverPointerT<model::AccountAddressNotification> CreateAccountAddressObserver();

	/// Observes account public key changes.
	NotificationObserverPointerT<model::AccountPublicKeyNotification> CreateAccountPublicKeyObserver();

	// endregion

	// region Block

	/// Observes block notifications and triggers importance recalculations using either \a pCommitCalculator (for commits)
	/// or \a pRollbackCalculator (for rollbacks).
	NotificationObserverPointerT<model::BlockNotification> CreateRecalculateImportancesObserver(
			std::unique_ptr<ImportanceCalculator>&& pCommitCalculator,
			std::unique_ptr<ImportanceCalculator>&& pRollbackCalculator);

	/// Observes block notifications and credits the harvester with transaction fees.
	NotificationObserverPointerT<model::BlockNotification> CreateHarvestFeeObserver();

	// endregion

	// region Transaction

	/// Observes balance changes triggered by balance transfer notifications.
	NotificationObserverPointerT<model::BalanceTransferNotification> CreateBalanceObserver();

	// endregion
}}
