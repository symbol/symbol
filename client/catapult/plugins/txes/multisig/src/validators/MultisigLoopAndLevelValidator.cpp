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
#include "src/cache/MultisigCache.h"
#include "src/cache/MultisigCacheUtils.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MultisigNewCosignatoryNotification;

	namespace {
		class LoopAndLevelChecker {
		public:
			LoopAndLevelChecker(const cache::MultisigCache::CacheReadOnlyType& multisigCache, uint8_t maxMultisigDepth)
					: m_multisigCache(multisigCache)
					, m_maxMultisigDepth(maxMultisigDepth)
			{}

		public:
			ValidationResult validate(const Address& top, const Address& bottom) {
				model::AddressSet ancestors;
				ancestors.insert(top);
				auto numTopLevels = FindAncestors(m_multisigCache, top, ancestors);

				model::AddressSet descendants;
				descendants.insert(bottom);
				auto numBottomLevels = FindDescendants(m_multisigCache, bottom, descendants);

				if (numTopLevels + numBottomLevels + 1 > m_maxMultisigDepth)
					return Failure_Multisig_Max_Multisig_Depth;

				auto hasLoop = std::any_of(ancestors.cbegin(), ancestors.cend(), [&descendants](const auto& address) {
					return descendants.cend() != descendants.find(address);
				});
				return hasLoop ? Failure_Multisig_Loop : ValidationResult::Success;
			}

		private:
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			uint8_t m_maxMultisigDepth;
		};
	}

	DECLARE_STATEFUL_VALIDATOR(MultisigLoopAndLevel, Notification)(uint8_t maxMultisigDepth) {
		return MAKE_STATEFUL_VALIDATOR(MultisigLoopAndLevel, [maxMultisigDepth](
					const Notification& notification,
					const ValidatorContext& context) {
			auto checker = LoopAndLevelChecker(context.Cache.sub<cache::MultisigCache>(), maxMultisigDepth);
			return checker.validate(notification.Multisig, context.Resolvers.resolve(notification.Cosignatory));
		});
	}
}}
