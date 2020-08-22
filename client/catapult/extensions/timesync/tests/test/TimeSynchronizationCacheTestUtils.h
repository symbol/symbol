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
#include "catapult/model/HeightGrouping.h"

namespace catapult {
	namespace cache { class AccountStateCacheDelta; }
	namespace ionet { class NodeContainer; }
}

namespace catapult { namespace test {

	/// Adds an account with \a publicKey, \a importance and \a importanceHeight to account state cache \a delta.
	void AddAccount(
			cache::AccountStateCacheDelta& delta,
			const Key& publicKey,
			Importance importance,
			model::ImportanceHeight importanceHeight);

	/// Adds a node with \a identityKey and \a nodeName to node \a container.
	void AddNode(ionet::NodeContainer& container, const Key& identityKey, const std::string& nodeName);
}}
