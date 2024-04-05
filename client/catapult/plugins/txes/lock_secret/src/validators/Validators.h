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
#include "src/model/SecretLockNotifications.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult { namespace validators {

	/// Validator that applies to secret lock notifications and validates that:
	/// - lock duration is at most \a maxSecretLockDuration
	DECLARE_STATELESS_VALIDATOR(SecretLockDuration, model::SecretLockDurationNotification)(BlockDuration maxSecretLockDuration);

	/// Validator that applies to secret lock hash algorithm notifications and validates that:
	/// - hash algorithm is valid
	DECLARE_STATELESS_VALIDATOR(SecretLockHashAlgorithm, model::SecretLockHashAlgorithmNotification)();

	/// Validator that applies to secret lock notifications and validates that:
	/// - attached hash is not present in secret lock info cache
	/// - validation is skipped at heights specified by \a skipHeights
	DECLARE_STATEFUL_VALIDATOR(SecretLockCacheUnique, model::SecretLockNotification)(
			const std::unordered_set<Height, utils::BaseValueHasher<Height>>& skipHeights);

	/// Validator that applies to proof notifications and validates that:
	/// - hash algorithm is supported
	/// - proof size is within inclusive bounds of \a minProofSize and \a maxProofSize
	/// - hash of proof matches secret
	DECLARE_STATELESS_VALIDATOR(ProofSecret, model::ProofSecretNotification)(uint16_t minProofSize, uint16_t maxProofSize);

	/// Validator that applies to proof notifications and validates that:
	/// - secret obtained from proof is present in cache
	DECLARE_STATEFUL_VALIDATOR(Proof, model::ProofPublicationNotification)();
}}
