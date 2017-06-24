#pragma once
#include "Results.h"
#include "src/model/AggregateNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// A validator implementation that applies to aggregate cosignatures notifications and validates that:
	/// - the number of transactions does not exceed \a maxTransactions
	/// - the number of implicit and explicit cosignatures does not exceed \a maxCosignatures
	/// - there are no redundant cosigners
	stateless::NotificationValidatorPointerT<model::AggregateCosignaturesNotification> CreateBasicAggregateCosignaturesValidator(
			uint32_t maxTransactions,
			uint8_t maxCosignatures);

	/// A validator implementation that applies to aggregate cosignatures notifications and validates that:
	/// - the set of component signers is equal to the set of cosigners
	stateless::NotificationValidatorPointerT<model::AggregateCosignaturesNotification> CreateStrictAggregateCosignaturesValidator();
}}
