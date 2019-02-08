/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "catapult/utils/TimeSpan.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	// region Address

	/// A validator implementation that applies to all account address notifications and validates that:
	/// - the address is valid and targets the expected network (\a networkIdentifier)
	DECLARE_STATEFUL_VALIDATOR(Address, model::AccountAddressNotification)(model::NetworkIdentifier networkIdentifier);

	// endregion

	// region VerifiableEntity

	/// A validator implementation that applies to all signature notifications and validates that:
	/// - nemesis account signatures are not allowed after the nemesis block
	DECLARE_STATEFUL_VALIDATOR(NemesisSink, model::SignatureNotification)();

	/// A validator implementation that applies to all entity notifications and validates that:
	/// - the entity targets the expected network (\a networkIdentifier)
	DECLARE_STATELESS_VALIDATOR(Network, model::EntityNotification)(model::NetworkIdentifier networkIdentifier);

	/// A validator implementation that applies to entity notifications and validates that:
	/// - the entity version is within supported range.
	DECLARE_STATELESS_VALIDATOR(EntityVersion, model::EntityNotification)();

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

	/// A validator implementation that applies to all balance debit notifications and validates that:
	/// - the sending account has enough funds
	DECLARE_STATEFUL_VALIDATOR(BalanceDebit, model::BalanceDebitNotification)();

	/// A validator implementation that applies to all transaction fee notifications and validates that:
	/// - fee is no greater than max fee
	/// - max fee multiplier does not overflow 32-bit value
	DECLARE_STATELESS_VALIDATOR(TransactionFee, model::TransactionFeeNotification)();

	// endregion
}}
