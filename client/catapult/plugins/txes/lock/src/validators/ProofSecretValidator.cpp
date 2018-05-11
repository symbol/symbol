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

#include "Validators.h"
#include "src/model/LockHashUtils.h"

namespace catapult { namespace validators {

	using Notification = model::ProofSecretNotification;

	namespace {
		constexpr bool SupportedHash(model::LockHashAlgorithm hashAlgorithm) {
			return hashAlgorithm == model::LockHashAlgorithm::Op_Sha3;
		}
	}

	DECLARE_STATELESS_VALIDATOR(ProofSecret, Notification)(uint16_t minProofSize, uint16_t maxProofSize) {
		return MAKE_STATELESS_VALIDATOR(ProofSecret, ([minProofSize, maxProofSize](const auto& notification) {
			if (!SupportedHash(notification.HashAlgorithm))
				return Failure_Lock_Hash_Not_Implemented;

			if (notification.Proof.Size < minProofSize || notification.Proof.Size > maxProofSize)
				return Failure_Lock_Proof_Size_Out_Of_Bounds;

			auto secret = model::CalculateHash(notification.HashAlgorithm, notification.Proof);
			return notification.Secret == secret ? ValidationResult::Success : Failure_Lock_Secret_Mismatch;
		}));
	}
}}
