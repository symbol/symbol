#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace unbondedpruning {

	/// Creates a registrar for an unbonded pruning service.
	/// \note This service is responsible for pruning unbonded aggregates from the pt cache.
	DECLARE_SERVICE_REGISTRAR(UnbondedPruning)();
}}
