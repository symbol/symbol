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
#include "catapult/ionet/NodeInfo.h"
#include "catapult/validators/ValidationResult.h"
#include "catapult/plugins.h"
#include "catapult/types.h"

namespace catapult {
	namespace ionet { class Node; }
	namespace model { struct NodeIdentity; }
}

namespace catapult { namespace subscribers {

	/// Node subscriber.
	class PLUGIN_API_DEPENDENCY NodeSubscriber {
	public:
		virtual ~NodeSubscriber() = default;

	public:
		/// Indicates a new \a node was found.
		virtual void notifyNode(const ionet::Node& node) = 0;

		/// Indicates a new incoming connection for node with \a identity connected to \a serviceId.
		virtual bool notifyIncomingNode(const model::NodeIdentity& identity, ionet::ServiceIdentifier serviceId) = 0;

		/// Indicates node with \a identity is banned due to \a reason.
		virtual void notifyBan(const model::NodeIdentity& identity, validators::ValidationResult reason) = 0;
	};
}}
