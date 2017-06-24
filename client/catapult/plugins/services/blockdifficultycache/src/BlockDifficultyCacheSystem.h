#pragma once

namespace catapult { namespace plugins { class PluginManager; } }

namespace catapult { namespace plugins {

	/// Registers the block difficulty cache system with \a manager.
	/// \note This plugin is required for basic system operation.
	void RegisterBlockDifficultyCacheSystem(PluginManager& manager);
}}
