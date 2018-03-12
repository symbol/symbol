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
