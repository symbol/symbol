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

#include "Validators.h"
#include "src/cache/SecretLockInfoCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ProofPublicationNotification;

	DEFINE_STATEFUL_VALIDATOR(Proof, [](const Notification& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::SecretLockInfoCache>();
		auto key = model::CalculateSecretLockInfoHash(notification.Secret, context.Resolvers.resolve(notification.Recipient));
		if (!cache.contains(key))
			return Failure_LockSecret_Unknown_Composite_Key;

		if (!cache.isActive(key, context.Height))
			return Failure_LockSecret_Inactive_Secret;

		auto lockInfoIter = cache.find(key);
		const auto& lockInfo = lockInfoIter.get().back();
		if (lockInfo.HashAlgorithm != notification.HashAlgorithm)
			return Failure_LockSecret_Hash_Algorithm_Mismatch;

		return ValidationResult::Success;
	})
}}
