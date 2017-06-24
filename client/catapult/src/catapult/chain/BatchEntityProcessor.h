#pragma once
#include "ExecutionConfiguration.h"

namespace catapult { namespace chain {

	/// Function signature for validating and executing a batch of entity infos with a shared height and time and updating
	/// stateful information.
	using BatchEntityProcessor = std::function<validators::ValidationResult (
			Height,
			Timestamp,
			const model::WeakEntityInfos&,
			const observers::ObserverState&)>;

	/// Creates a batch entity processor around \a config.
	BatchEntityProcessor CreateBatchEntityProcessor(const ExecutionConfiguration& config);
}}
