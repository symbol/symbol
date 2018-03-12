#pragma once
#include "Results.h"
#include "src/model/AggregateNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// A validator implementation that applies to aggregate cosignatures notifications and validates that:
	/// - the number of transactions does not exceed \a maxTransactions
	/// - the number of implicit and explicit cosignatures does not exceed \a maxCosignatures
	/// - there are no redundant cosigners
	DECLARE_STATELESS_VALIDATOR(BasicAggregateCosignatures, model::AggregateCosignaturesNotification)(
			uint32_t maxTransactions,
			uint8_t maxCosignatures);

	/// A validator implementation that applies to aggregate cosignatures notifications and validates that:
	/// - the set of component signers is equal to the set of cosigners
	DECLARE_STATELESS_VALIDATOR(StrictAggregateCosignatures, model::AggregateCosignaturesNotification)();
}}
