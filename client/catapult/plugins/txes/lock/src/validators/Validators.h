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
#include "src/model/LockNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// A validator implementation that applies to hash lock notifications and validates that:
	/// - lock duration is at most \a maxHashLockDuration
	DECLARE_STATELESS_VALIDATOR(HashLockDuration, model::HashLockDurationNotification)(BlockDuration maxHashLockDuration);

	/// A validator implementation that applies to secret lock notifications and validates that:
	/// - lock duration is at most \a maxSecretLockDuration
	DECLARE_STATELESS_VALIDATOR(SecretLockDuration, model::SecretLockDurationNotification)(BlockDuration maxSecretLockDuration);

	/// A validator implementation that applies to secret lock hash algorithm notifications and validates that:
	/// - hash algorithm is valid
	DECLARE_STATELESS_VALIDATOR(SecretLockHashAlgorithm, model::SecretLockHashAlgorithmNotification)();

	/// A validator implementation that applies to hash lock mosaic notifications and validates that:
	/// - mosaic id is nem.xem
	/// - mosaic amount is equal to \a lockedFundsPerAggregate
	DECLARE_STATELESS_VALIDATOR(HashLockMosaic, model::HashLockMosaicNotification)(Amount lockedFundsPerAggregate);

	/// A validator implementation that applies to hash lock notifications and validates that:
	/// - attached hash is not present in hash lock info cache
	DECLARE_STATEFUL_VALIDATOR(HashCacheUnique, model::HashLockNotification)();

	/// A validator implementation that applies to secret lock notifications and validates that:
	/// - attached hash is not present in secret lock info cache
	DECLARE_STATEFUL_VALIDATOR(SecretCacheUnique, model::SecretLockNotification)();

	/// A validator implementation that applies to proof notifications and validates that:
	/// - hash algorithm is supported
	/// - proof size is within inclusive bounds of \a minProofSize and \a maxProofSize
	/// - hash of proof matches secret
	DECLARE_STATELESS_VALIDATOR(ProofSecret, model::ProofSecretNotification)(uint16_t minProofSize, uint16_t maxProofSize);

	/// A validator implementation that applies to proof notifications and validates that:
	/// - secret obtained from proof is present in cache
	DECLARE_STATEFUL_VALIDATOR(Proof, model::ProofPublicationNotification)();

	/// A validator implementation that applies to transaction notifications and validates that:
	/// - incomplete aggregate transactions must have an active, unused hash lock info present in cache
	DECLARE_STATEFUL_VALIDATOR(AggregateHashPresent, model::TransactionNotification)();
}}
