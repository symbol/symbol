#include "Validators.h"
#include "src/cache/MultisigCache.h"
#include "src/cache/MultisigCacheUtils.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigNewCosignerNotification;

	namespace {
		class LoopAndLevelChecker {
		public:
			explicit LoopAndLevelChecker(const cache::MultisigCache::CacheReadOnlyType& multisigCache, uint8_t maxMultisigDepth)
					: m_multisigCache(multisigCache)
					, m_maxMultisigDepth(maxMultisigDepth)
			{}

		public:
			ValidationResult validate(const Key& topKey, const Key& bottomKey) {
				utils::KeySet ancestorKeys;
				ancestorKeys.insert(topKey);
				auto numTopLevels = FindAncestors(m_multisigCache, topKey, ancestorKeys);

				utils::KeySet descendantKeys;
				descendantKeys.insert(bottomKey);
				auto numBottomLevels = FindDescendants(m_multisigCache, bottomKey, descendantKeys);

				if (numTopLevels + numBottomLevels + 1 > m_maxMultisigDepth)
					return Failure_Multisig_Modify_Max_Multisig_Depth;

				auto hasLoop = std::any_of(ancestorKeys.cbegin(), ancestorKeys.cend(), [&descendantKeys](const auto& key) {
					return descendantKeys.cend() != descendantKeys.find(key);
				});
				return hasLoop ? Failure_Multisig_Modify_Loop : ValidationResult::Success;
			}

		private:
			const cache::MultisigCache::CacheReadOnlyType& m_multisigCache;
			uint8_t m_maxMultisigDepth;
		};
	}

	DECLARE_STATEFUL_VALIDATOR(ModifyMultisigLoopAndLevel, Notification)(uint8_t maxMultisigDepth) {
		return MAKE_STATEFUL_VALIDATOR(ModifyMultisigLoopAndLevel, [maxMultisigDepth](
					const auto& notification,
					const ValidatorContext& context) {
			auto checker = LoopAndLevelChecker(context.Cache.sub<cache::MultisigCache>(), maxMultisigDepth);
			return checker.validate(notification.MultisigAccountKey, notification.CosignatoryKey);
		});
	}
}}
