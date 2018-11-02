/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "catapult/plugins/PluginManager.h"
#include "catapult/validators/ValidatorTypes.h"
#include <memory>

namespace catapult {
	namespace config { class LocalNodeConfiguration; }
	namespace observers { class EntityObserver; }
}

namespace catapult { namespace extensions {

	/// Creates plugin storage configuration from \a config.
	plugins::StorageConfiguration CreateStorageConfiguration(const config::LocalNodeConfiguration& config);

	/// Creates an entity stateless validator using \a pluginManager.
	std::unique_ptr<const validators::stateless::AggregateEntityValidator> CreateStatelessValidator(const plugins::PluginManager& manager);

	/// Creates an entity observer using \a pluginManager.
	std::unique_ptr<const observers::EntityObserver> CreateEntityObserver(const plugins::PluginManager& manager);

	/// Creates an undo entity observer using \a pluginManager.
	std::unique_ptr<const observers::EntityObserver> CreateUndoEntityObserver(const plugins::PluginManager& manager);

	/// Creates a permanent entity observer using \a pluginManager.
	std::unique_ptr<const observers::EntityObserver> CreatePermanentEntityObserver(const plugins::PluginManager& manager);
}}
