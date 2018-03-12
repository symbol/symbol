#pragma once
#include "Results.h"
#include "src/model/MultisigNotifications.h"
#include "plugins/txes/aggregate/src/model/AggregateNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// A validator implementation that applies to modify multisig cosigners notifications and validates that:
	/// - same account does not occur in removed and added cosignatories
	/// - there is at most one cosignatory removed
	stateless::NotificationValidatorPointerT<model::ModifyMultisigCosignersNotification>
	CreateModifyMultisigCosignersValidator();

	/// A validator implementation that applies to transaction notifications and validates that:
	/// - multisig accounts cannot initiate transactions
	stateful::NotificationValidatorPointerT<model::TransactionNotification>
	CreateMultisigPermittedOperationValidator();

	/// A validator implementation that applies to modify multisig cosigners notifications and validates that:
	/// - added account isn't already a cosignatory
	/// - removed account is already a cosignatory
	stateful::NotificationValidatorPointerT<model::ModifyMultisigCosignersNotification>
	CreateModifyMultisigInvalidCosignersValidator();

	/// A validator implementation that applies to modify multisig new cosigner notifications and validates that:
	/// - the cosignatory is cosigning at most \a maxCosignedAccountsPerAccount
	stateful::NotificationValidatorPointerT<model::ModifyMultisigNewCosignerNotification>
	CreateModifyMultisigMaxCosignedAccountsValidator(uint8_t maxCosignedAccountsPerAccount);

	/// A validator implementation that applies to modify multisig cosigners notifications and validates that:
	/// - the multisig account has at most \a maxCosignersPerAccount cosignatories
	stateful::NotificationValidatorPointerT<model::ModifyMultisigCosignersNotification>
	CreateModifyMultisigMaxCosignersValidator(uint8_t maxCosignersPerAccount);

	/// A validator implementation that applies to modify multisig new cosigner notifications and validates that:
	/// - the multisig depth is at most \a maxMultisigDepth
	/// - no multisig loops are created
	stateful::NotificationValidatorPointerT<model::ModifyMultisigNewCosignerNotification>
	CreateModifyMultisigLoopAndLevelValidator(uint8_t maxMultisigDepth);

	/// A validator implementation that applies to modify multisig settings notifications and validates that:
	/// - new min removal and min approval are greater than 0
	/// - new min removal and min approval settings are not greater than total number of cosignatories
	stateful::NotificationValidatorPointerT<model::ModifyMultisigSettingsNotification>
	CreateModifyMultisigInvalidSettingsValidator();

	/// A validator implementation that applies to aggregate cosignatures notifications and validates that:
	///  - all cosigners are eligible counterparties
	stateful::NotificationValidatorPointerT<model::AggregateCosignaturesNotification>
	CreateMultisigAggregateEligibleCosignersValidator();

	/// A validator implementation that applies to aggregate embeded transaction notifications and validates that:
	///  - present cosigners are sufficient
	stateful::NotificationValidatorPointerT<model::AggregateEmbeddedTransactionNotification>
	CreateMultisigAggregateSufficientCosignersValidator();
}}
