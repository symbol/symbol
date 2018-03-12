#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace sync {

	/// Creates a registrar for a dispatcher service.
	/// \note This service is responsible for enabling node block and transaction processing.
	DECLARE_SERVICE_REGISTRAR(Dispatcher)();
}}
