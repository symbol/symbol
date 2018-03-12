#pragma once
#include "src/model/MultisigNotifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace observers {

	/// Observes changes triggered by modify multisig cosigners notifications, and:
	/// - adds / deletes multisig account to / from cache
	/// - adds / deletes cosignatories
	DECLARE_OBSERVER(ModifyMultisigCosigners, model::ModifyMultisigCosignersNotification)();

	/// Observes changes triggered by modify multisig settings notifications, and:
	/// - sets new values of min removal and min approval
	DECLARE_OBSERVER(ModifyMultisigSettings, model::ModifyMultisigSettingsNotification)();
}}
