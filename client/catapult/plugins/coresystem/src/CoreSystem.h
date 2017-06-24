#pragma once

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace plugins {

	/// Registers the core system with \a manager.
	/// \note This plugin is required for basic system operation.
	void RegisterCoreSystem(PluginManager& manager);
}}
