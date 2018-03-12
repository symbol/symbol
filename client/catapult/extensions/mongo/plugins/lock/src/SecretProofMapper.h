#pragma once
#include "mongo/src/MongoTransactionPlugin.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Creates a mongo secret proof transaction plugin.
	PLUGIN_API
	std::unique_ptr<MongoTransactionPlugin> CreateSecretProofTransactionMongoPlugin();
}}}
