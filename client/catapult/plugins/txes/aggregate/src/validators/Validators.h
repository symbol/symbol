/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "Results.h"
#include "src/model/AggregateNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// Validator that applies to aggregate cosignatures notifications and validates that:
	/// - the number of transactions does not exceed \a maxTransactions
	/// - the number of implicit and explicit cosignatures does not exceed \a maxCosignatures
	/// - there are no redundant cosignatories
	DECLARE_STATELESS_VALIDATOR(BasicAggregateCosignatures, model::AggregateCosignaturesNotification)(
			uint32_t maxTransactions,
			uint8_t maxCosignatures);

	/// Validator that applies to aggregate cosignatures notifications and validates that:
	/// - the set of component signers is equal to the set of cosignatories
	DECLARE_STATELESS_VALIDATOR(StrictAggregateCosignatures, model::AggregateCosignaturesNotification)();

	/// Validator that applies to aggregate embedded transactions notifications and validates that:
	/// - the aggregate transactions hash is correct
	DECLARE_STATELESS_VALIDATOR(AggregateTransactionsHash, model::AggregateEmbeddedTransactionsNotification)();
}}
