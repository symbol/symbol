#pragma once
#include "src/model/LockNotifications.h"
#include "catapult/model/Notifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes changes triggered by hash lock notifications, and:
	/// - adds/removes hash lock info to/from hash lock info cache
	/// - debits/credits lock owner
	DECLARE_OBSERVER(HashLock, model::HashLockNotification)();

	/// Observes changes triggered by secret lock notifications, and:
	/// - adds/removes secret lock info to/from secret lock info cache
	/// - debits/credits lock owner
	DECLARE_OBSERVER(SecretLock, model::SecretLockNotification)();

	/// Observes changes triggered by proof notifications, and:
	/// - credits/debits proof publisher
	/// - marks proper secret lock as used/unused
	DECLARE_OBSERVER(Proof, model::ProofPublicationNotification)();

	/// Observes hashes of completed, bonded aggregate transactions, and:
	/// - credits/debits lock owner
	/// - marks proper hash lock as used/unused
	DECLARE_OBSERVER(CompletedAggregate, model::TransactionNotification)();

	/// Observes block notifications and triggers handling of expired hash lock infos, including:
	/// - crediting the block signer the mosaics given in the lock info
	DECLARE_OBSERVER(ExpiredHashLockInfo, model::BlockNotification)();

	/// Observes block notifications and triggers handling of expired secret lock infos, including:
	/// - crediting the lock creator the mosaics given in the lock info
	DECLARE_OBSERVER(ExpiredSecretLockInfo, model::BlockNotification)();
}}
