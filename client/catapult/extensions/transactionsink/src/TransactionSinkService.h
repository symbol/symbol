#pragma once
#include "catapult/extensions/ServiceRegistrar.h"

namespace catapult { namespace transactionsink {

	/// Creates a registrar for a transaction sink service.
	/// \note This service is responsible for allowing the node to accept transactions.
	DECLARE_SERVICE_REGISTRAR(TransactionSink)();
}}
