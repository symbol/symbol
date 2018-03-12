#pragma once
#include "catapult/validators/ValidatorTypes.h"
#include <memory>

namespace catapult {
	namespace observers { class EntityObserver; }
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace extensions {

	/// Creates an entity stateless validator using \a pluginManager.
	std::unique_ptr<const validators::stateless::AggregateEntityValidator> CreateStatelessValidator(const plugins::PluginManager& manager);

	/// Creates an entity observer using \a pluginManager.
	std::unique_ptr<const observers::EntityObserver> CreateEntityObserver(const plugins::PluginManager& manager);

	/// Creates an undo entity observer using \a pluginManager.
	std::unique_ptr<const observers::EntityObserver> CreateUndoEntityObserver(const plugins::PluginManager& manager);

	/// Creates a permanent entity observer using \a pluginManager.
	std::unique_ptr<const observers::EntityObserver> CreatePermanentEntityObserver(const plugins::PluginManager& manager);
}}
