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
#include "catapult/utils/NonCopyable.h"

namespace catapult {
	namespace extensions {
		struct LocalNodeStateConstRef;
		struct LocalNodeStateRef;
	}
	namespace plugins { class PluginManager; }
}

namespace catapult { namespace extensions {

	/// An interface for saving and loading block chains.
	class BlockChainStorage : public utils::NonCopyable {
	public:
		virtual ~BlockChainStorage() {}

	public:
		/// Loads data from storage into \a stateRef using plugins registered with \a pluginManager.
		virtual void loadFromStorage(const LocalNodeStateRef& stateRef, const plugins::PluginManager& pluginManager) = 0;

		/// Saves all in memory state from \a stateRef to storage.
		virtual void saveToStorage(const LocalNodeStateConstRef& stateRef) = 0;
	};
}}
