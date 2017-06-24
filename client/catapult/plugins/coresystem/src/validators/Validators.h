#pragma once
#include "Results.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	// region VerifiableEntity

	/// A validator implementation that applies to all signature notifications and validates that:
	/// - nemesis account signatures are not allowed after the nemesis block
	stateful::NotificationValidatorPointerT<model::SignatureNotification> CreateNemesisSinkValidator();

	/// A validator implementation that applies to all signature notifications and validates that:
	/// - signatures are valid
	stateless::NotificationValidatorPointerT<model::SignatureNotification> CreateSignatureValidator();

	/// A validator implementation that applies to all entity notifications and validates that:
	/// - the entity targets the expected network
	stateless::NotificationValidatorPointerT<model::EntityNotification> CreateNetworkValidator(
			model::NetworkIdentifier networkIdentifier);

	// endregion

	// region Block

	/// A validator implementation that applies to all block notifications and validates that:
	/// - the block timestamp is not too far in the future (less than \a maxBlockFutureTime ahead)
	stateless::NotificationValidatorPointerT<model::BlockNotification> CreateNonFutureBlockValidator(
			const utils::TimeSpan& maxBlockFutureTime);

	/// A validator implementation that applies to all block notifications and validates that:
	/// - the block signer was eligible to create the block given the minimum balance required to harvest a block
	///   (\a minHarvesterBalance)
	stateful::NotificationValidatorPointerT<model::BlockNotification> CreateEligibleHarvesterValidator(Amount minHarvesterBalance);

	/// A validator implementation that applies to all block notifications and validates that:
	/// - the block does not contain more than \a maxTransactions transactions
	stateless::NotificationValidatorPointerT<model::BlockNotification> CreateMaxTransactionsValidator(uint32_t maxTransactions);

	// endregion

	// region Transaction

	/// A validator implementation that applies to all transaction notifications and validates that:
	/// - the transaction deadline is no later than the block timestamp
	/// - the transaction deadline is no more than \a maxTransactionLifetime past the block timestamp
	stateful::NotificationValidatorPointerT<model::TransactionNotification> CreateDeadlineValidator(
			const utils::TimeSpan& maxTransactionLifetime);

	/// A validator implementation that applies to all balance transfer notifications and validates that:
	/// - the sending account has enough funds
	stateful::NotificationValidatorPointerT<model::BalanceTransferNotification> CreateBalanceValidator();

	// endregion
}}
