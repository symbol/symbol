#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace diagnostics {

	/// Creates a registrar for a diagnostics service.
	/// \note This service is responsible for enabling node diagnostics.
	DECLARE_SERVICE_REGISTRAR(Diagnostics)();
}}
