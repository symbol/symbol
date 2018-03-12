#pragma once
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
			locator.service<net::PacketWriters>(serviceName)->broadcast(payload);
		};
	}

	/// Creates a sink that pushes entities using \a packetType and a service identified by \a serviceName in \a locator.
	template<typename TSink>
	TSink CreatePushEntitySink(const extensions::ServiceLocator& locator, const std::string& serviceName, ionet::PacketType packetType) {
		return [&locator, serviceName, packetType](const auto& entities) {
			auto payload = ionet::CreateBroadcastPayload(entities, packetType);
			locator.service<net::PacketWriters>(serviceName)->broadcast(payload);
		};
	}
}}
