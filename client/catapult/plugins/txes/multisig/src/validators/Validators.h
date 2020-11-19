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
#include "src/model/MultisigNotifications.h"
#include "plugins/txes/aggregate/src/model/AggregateNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// Validator that applies to multisig cosignatories notifications and validates that:
	/// - same account does not occur in removed and added cosignatories
	/// - there is at most one cosignatory removed
	DECLARE_STATELESS_VALIDATOR(MultisigCosignatories, model::MultisigCosignatoriesNotification)();

	/// Validator that applies to transaction notifications and validates that:
	/// - multisig accounts cannot initiate transactions
	DECLARE_STATEFUL_VALIDATOR(MultisigPermittedOperation, model::TransactionNotification)();

	/// Validator that applies to multisig cosignatories notifications and validates that:
	/// - added account isn't already a cosignatory
	/// - removed account is already a cosignatory
	DECLARE_STATEFUL_VALIDATOR(MultisigInvalidCosignatories, model::MultisigCosignatoriesNotification)();

	/// Validator that applies to multisig new osignatory notifications and validates that:
	/// - the cosignatory is cosigning at most \a maxCosignedAccountsPerAccount
	DECLARE_STATEFUL_VALIDATOR(MultisigMaxCosignedAccounts, model::MultisigNewCosignatoryNotification)(
			uint32_t maxCosignedAccountsPerAccount);

	/// Validator that applies to multisig cosignatories notifications and validates that:
	/// - the multisig account has at most \a maxCosignatoriesPerAccount cosignatories
	DECLARE_STATEFUL_VALIDATOR(MultisigMaxCosignatories, model::MultisigCosignatoriesNotification)(uint32_t maxCosignatoriesPerAccount);

	/// Validator that applies to multisig new cosignatory notifications and validates that:
	/// - the multisig depth is at most \a maxMultisigDepth
	/// - no multisig loops are created
	DECLARE_STATEFUL_VALIDATOR(MultisigLoopAndLevel, model::MultisigNewCosignatoryNotification)(uint8_t maxMultisigDepth);

	/// Validator that applies to multisig settings notifications and validates that:
	/// - new min removal and min approval are greater than 0
	/// - new min removal and min approval settings are not greater than total number of cosignatories
	DECLARE_STATEFUL_VALIDATOR(MultisigInvalidSettings, model::MultisigSettingsNotification)();

	/// Validator that applies to aggregate cosignatures notifications and validates that:
	///  - all cosignatories are eligible counterparties (using \a transactionRegistry to retrieve custom approval requirements)
	DECLARE_STATEFUL_VALIDATOR(
			MultisigAggregateEligibleCosignatories,
			model::AggregateCosignaturesNotification)(const model::TransactionRegistry& transactionRegistry);

	/// Validator that applies to aggregate embeded transaction notifications and validates that:
	///  - present cosignatories are sufficient (using \a transactionRegistry to retrieve custom approval requirements)
	DECLARE_STATEFUL_VALIDATOR(
			MultisigAggregateSufficientCosignatories,
			model::AggregateEmbeddedTransactionNotification)(const model::TransactionRegistry& transactionRegistry);
}}
