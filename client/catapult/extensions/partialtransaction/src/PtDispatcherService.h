#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace partialtransaction {

	/// Creates a registrar for a partial transaction dispatcher service.
	DECLARE_SERVICE_REGISTRAR(PtDispatcher)();
}}
