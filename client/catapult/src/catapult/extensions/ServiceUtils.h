/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "ServerHooks.h"
#include "ServiceLocator.h"
#include "catapult/ionet/BroadcastUtils.h"
#include "catapult/net/PacketWriters.h"
#include <string>

namespace catapult { namespace extensions {

	/// Creates a sink that pushes entities using a service identified by \a serviceName in \a locator.
	template<typename TSink>
	TSink CreatePushEntitySink(const extensions::ServiceLocator& locator, const std::string& serviceName) {
		return [&locator, serviceName](const auto& entities) {
			auto payload = ionet::CreateBroadcastPayload(entities);
			if (sizeof(ionet::PacketHeader) == payload.header().Size)
				return;

			locator.service<net::PacketWriters>(serviceName)->broadcast(payload);
		};
	}

	/// Creates a sink that pushes entities using \a packetType and a service identified by \a serviceName in \a locator.
	template<typename TSink>
	TSink CreatePushEntitySink(const extensions::ServiceLocator& locator, const std::string& serviceName, ionet::PacketType packetType) {
		return [&locator, serviceName, packetType](const auto& entities) {
			auto payload = ionet::CreateBroadcastPayload(entities, packetType);
			if (sizeof(ionet::PacketHeader) == payload.header().Size)
				return;

			locator.service<net::PacketWriters>(serviceName)->broadcast(payload);
		};
	}

	/// Creates a sink that closes the propagated node identity in \a container.
	inline BannedNodeIdentitySink CreateCloseConnectionSink(net::ConnectionContainer& container) {
		return [&container](const auto& identity) {
			container.closeOne(identity);
		};
	}
}}
