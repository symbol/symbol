#pragma once
#include "catapult/chain/ChainFunctions.h"
#include "catapult/validators/ValidatorTypes.h"

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace chain {

	/// Creates a joint validator around \a cache and current time supplier (\a timeSupplier) using \a pluginManager
	/// that ignores suppressed failures according to \a isSuppressedFailure.
	std::unique_ptr<const validators::stateless::NotificationValidator> CreateJointValidator(
			const cache::CatapultCache& cache,
			const TimeSupplier& timeSupplier,
			const plugins::PluginManager& pluginManager,
			const validators::ValidationResultPredicate& isSuppressedFailure);
}}
