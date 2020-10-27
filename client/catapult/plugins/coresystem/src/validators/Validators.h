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
#include "catapult/utils/TimeSpan.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	// region Address / Key

	/// Validator that applies to all account address notifications and validates that:
	/// - the address is valid and targets the expected network
	DECLARE_STATEFUL_VALIDATOR(Address, model::AccountAddressNotification)();

	/// Validator that applies to all account public key notifications and validates that:
	/// - the public key is associated with a unique address (i.e. it does not collide with a previously registered public key)
	DECLARE_STATEFUL_VALIDATOR(PublicKey, model::AccountPublicKeyNotification)();

	/// Validator that applies to all account address notifications and validates that:
	/// - the address is nonzero given the expected network (\a networkIdentifier)
	DECLARE_STATELESS_VALIDATOR(ZeroAddress, model::AccountAddressNotification)(model::NetworkIdentifier networkIdentifier);

	/// Validator that applies to all account public key notifications and validates that:
	/// - the public key is nonzero
	DECLARE_STATELESS_VALIDATOR(ZeroPublicKey, model::AccountPublicKeyNotification)();

	// endregion

	// region VerifiableEntity

	/// Validator that applies to all signature notifications and validates that:
	/// - nemesis account signatures are not allowed after the nemesis block
	DECLARE_STATEFUL_VALIDATOR(NemesisSink, model::SignatureNotification)();

	/// Validator that applies to all entity notifications and validates that:
	/// - the entity targets the expected network (\a networkIdentifier)
	DECLARE_STATELESS_VALIDATOR(Network, model::EntityNotification)(model::NetworkIdentifier networkIdentifier);

	/// Validator that applies to entity notifications and validates that:
	/// - the entity version is within supported range.
	DECLARE_STATELESS_VALIDATOR(EntityVersion, model::EntityNotification)();

	// endregion

	// region Block / ImportanceBlock

	/// Validator that applies to all block notifications and validates that:
	/// - block type matches expected block type given \a importanceGrouping
	DECLARE_STATELESS_VALIDATOR(BlockType, model::BlockTypeNotification)(uint64_t importanceGrouping);

	/// Validator that applies to all block notifications and validates that:
	/// - the block signer was eligible to create the block
	DECLARE_STATEFUL_VALIDATOR(EligibleHarvester, model::BlockNotification)();

	/// Validator that applies to all block notifications and validates that:
	/// - the block does not contain more than \a maxTransactions transactions
	DECLARE_STATELESS_VALIDATOR(MaxTransactions, model::BlockNotification)(uint32_t maxTransactions);

	/// Validator that applies to all importance block notifications and validates that:
	/// - notification values match calculated values (excluding PreviousImportanceBlockHash, which is validated separately)
	DECLARE_STATEFUL_VALIDATOR(ImportanceBlock, model::ImportanceBlockNotification)();

	// endregion

	// region Transaction

	/// Validator that applies to all transaction notifications and validates that:
	/// - the transaction deadline is no later than the block timestamp
	/// - the transaction deadline is no more than \a maxTransactionLifetime past the block timestamp
	DECLARE_STATEFUL_VALIDATOR(Deadline, model::TransactionDeadlineNotification)(const utils::TimeSpan& maxTransactionLifetime);

	/// Validator that applies to all balance transfer notifications and validates that:
	/// - the sending account has enough funds
	DECLARE_STATEFUL_VALIDATOR(BalanceTransfer, model::BalanceTransferNotification)();

	/// Validator that applies to all balance debit notifications and validates that:
	/// - the sending account has enough funds
	DECLARE_STATEFUL_VALIDATOR(BalanceDebit, model::BalanceDebitNotification)();

	/// Validator that applies to all transaction fee notifications and validates that:
	/// - fee is no greater than max fee
	/// - max fee multiplier does not overflow 32-bit value
	DECLARE_STATELESS_VALIDATOR(TransactionFee, model::TransactionFeeNotification)();

	// endregion

	// region Other

	/// Validator that applies to all internal padding notifications and validates that:
	/// - internal padding is zero
	DECLARE_STATELESS_VALIDATOR(ZeroInternalPadding, model::InternalPaddingNotification)();

	// Validator that introduces first testnet fork at \a forkHeight and validates that:
	// - before fork block only v1 VotingKeyLink transactions are allowed
	// - after fork block only v2 VotingKeyLink transactions are allowed
	DECLARE_STATEFUL_VALIDATOR(EntityForkVersion, model::EntityNotification)(Height forkHeight);

	// endregion
}}
