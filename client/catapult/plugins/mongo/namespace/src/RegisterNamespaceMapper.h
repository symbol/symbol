#pragma once
#include "plugins/mongo/coremongo/src/MongoTransactionPlugin.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Creates a mongo register namespace transaction plugin.
	PLUGIN_API
	std::unique_ptr<MongoTransactionPlugin> CreateRegisterNamespaceTransactionMongoPlugin();
}}}
