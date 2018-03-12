#pragma once

namespace catapult { namespace mongo { class MongoPluginManager; } }

namespace catapult { namespace mongo {

	/// Registers the mongo core system with \a manager.
	/// \note This plugin is required for database operation.
	void RegisterCoreMongoSystem(MongoPluginManager& manager);
}}
