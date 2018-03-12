#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace partialtransaction {

	/// Creates a registrar for a partial transactions service.
	/// \note This service is responsible for sending partial transactions between api nodes.
	DECLARE_SERVICE_REGISTRAR(Pt)();
}}
