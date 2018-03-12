#pragma once
#include "mongo/src/MongoTransactionPlugin.h"

namespace catapult { namespace mongo { namespace plugins {

	/// Creates a mongo mosaic definition transaction plugin.
	PLUGIN_API
	std::unique_ptr<MongoTransactionPlugin> CreateMosaicDefinitionTransactionMongoPlugin();
}}}
