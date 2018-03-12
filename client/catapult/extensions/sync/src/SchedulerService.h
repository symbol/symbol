#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace sync { struct TasksConfiguration; } }

namespace catapult { namespace sync {

	/// Creates a registrar for a scheduler service around \a config
	/// \note This service is responsible for executing scheduled tasks.
	DECLARE_SERVICE_REGISTRAR(Scheduler)(const TasksConfiguration& config);
}}
