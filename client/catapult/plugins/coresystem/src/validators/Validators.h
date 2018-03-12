#pragma once
#include "Results.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	// region Address

	/// A validator implementation that applies to all account address notifications and validates that:
	/// - the address is valid and targets the expected network (\a networkIdentifier)
	DECLARE_STATELESS_VALIDATOR(Address, model::AccountAddressNotification)(model::NetworkIdentifier networkIdentifier);

	// endregion

	// region VerifiableEntity

	/// A validator implementation that applies to all signature notifications and validates that:
	/// - nemesis account signatures are not allowed after the nemesis block
	DECLARE_STATEFUL_VALIDATOR(NemesisSink, model::SignatureNotification)();

	/// A validator implementation that applies to all entity notifications and validates that:
	/// - the entity targets the expected network (\a networkIdentifier)
	DECLARE_STATELESS_VALIDATOR(Network, model::EntityNotification)(model::NetworkIdentifier networkIdentifier);

	// endregion

	// region Block

	/// A validator implementation that applies to all block notifications and validates that:
	/// - the block signer was eligible to create the block given the minimum balance required to harvest a block
	///   (\a minHarvesterBalance)
	DECLARE_STATEFUL_VALIDATOR(EligibleHarvester, model::BlockNotification)(Amount minHarvesterBalance);

	/// A validator implementation that applies to all block notifications and validates that:
	/// - the block does not contain more than \a maxTransactions transactions
	DECLARE_STATELESS_VALIDATOR(MaxTransactions, model::BlockNotification)(uint32_t maxTransactions);

	// endregion

	// region Transaction

	/// A validator implementation that applies to all transaction notifications and validates that:
	/// - the transaction deadline is no later than the block timestamp
	/// - the transaction deadline is no more than \a maxTransactionLifetime past the block timestamp
	DECLARE_STATEFUL_VALIDATOR(Deadline, model::TransactionNotification)(const utils::TimeSpan& maxTransactionLifetime);

	/// A validator implementation that applies to all balance transfer notifications and validates that:
	/// - the sending account has enough funds
	DECLARE_STATEFUL_VALIDATOR(BalanceTransfer, model::BalanceTransferNotification)();

	/// A validator implementation that applies to all balance reserve notifications and validates that:
	/// - the sending account has enough funds
	DECLARE_STATEFUL_VALIDATOR(BalanceReserve, model::BalanceReserveNotification)();

	// endregion
}}
