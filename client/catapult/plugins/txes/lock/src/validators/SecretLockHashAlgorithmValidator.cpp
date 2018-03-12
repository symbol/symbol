#include "Validators.h"

namespace catapult { namespace validators {

	using Notification = model::SecretLockHashAlgorithmNotification;

	namespace {
		bool IsValidHashAlgorithm(model::LockHashAlgorithm hashAlgorithm) {
			return utils::to_underlying_type(hashAlgorithm) <= utils::to_underlying_type(model::LockHashAlgorithm::Op_Hash_256);
		}
	}

	DEFINE_STATELESS_VALIDATOR(SecretLockHashAlgorithm, [](const auto& notification) {
		return IsValidHashAlgorithm(notification.HashAlgorithm) ? ValidationResult::Success : Failure_Lock_Invalid_Hash_Algorithm;
	});
}}
