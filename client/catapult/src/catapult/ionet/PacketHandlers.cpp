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

#include "PacketHandlers.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace ionet {

	// region ServerPacketHandlerContext

	ServerPacketHandlerContext::ServerPacketHandlerContext()
			: m_pKey(nullptr)
			, m_pHost(nullptr)
			, m_hasResponse(false)
			, m_defaultKey()
	{}

	ServerPacketHandlerContext::ServerPacketHandlerContext(const Key& key, const std::string& host)
			: m_pKey(&key)
			, m_pHost(&host)
			, m_hasResponse(false)
	{}

	const Key& ServerPacketHandlerContext::key() const {
		return !m_pKey ? m_defaultKey : *m_pKey;
	}

	const std::string& ServerPacketHandlerContext::host() const {
		return !m_pHost ? m_defaultHost : *m_pHost;
	}

	bool ServerPacketHandlerContext::hasResponse() const {
		return m_hasResponse;
	}

	const PacketPayload& ServerPacketHandlerContext::response() const {
		if (!hasResponse())
			CATAPULT_THROW_RUNTIME_ERROR("response is not set");

		return m_payload;
	}

	void ServerPacketHandlerContext::response(PacketPayload&& payload) {
		if (hasResponse())
			CATAPULT_THROW_RUNTIME_ERROR("response is already set");

		m_payload = std::move(payload);
		m_hasResponse = true;
	}

	// endregion

	// region ServerPacketHandlers

	ServerPacketHandlers::ServerPacketHandlers(uint32_t maxPacketDataSize) : m_maxPacketDataSize(maxPacketDataSize)
	{}

	size_t ServerPacketHandlers::size() const {
		size_t numHandlers = 0;
		for (const auto& descriptor : m_descriptors)
			numHandlers += descriptor.Handler ? 1 : 0;

		return numHandlers;
	}

	uint32_t ServerPacketHandlers::maxPacketDataSize() const {
		return m_maxPacketDataSize;
	}

	bool ServerPacketHandlers::canProcess(PacketType type) const {
		Packet packet;
		packet.Type = type;
		return canProcess(packet);
	}

	bool ServerPacketHandlers::canProcess(const Packet& packet) const {
		return !!findDescriptor(packet);
	}

	namespace {
		bool IsHostAllowed(const std::unordered_set<std::string>& hosts, const std::string& host) {
			if (hosts.empty())
				return true;

			return std::any_of(hosts.cbegin(), hosts.cend(), [&host](const auto& allowedHost) {
				return allowedHost == host;
			});
		}
	}

	bool ServerPacketHandlers::process(const Packet& packet, ContextType& context) const {
		const auto* pDescriptor = findDescriptor(packet);
		if (!pDescriptor)
			return false;

		if (!IsHostAllowed(pDescriptor->AllowedHosts, context.host())) {
			CATAPULT_LOG(warning) << "rejecting " << packet << " from " << context.host();
			return false;
		}

		CATAPULT_LOG(trace) << "processing " << packet;
		pDescriptor->Handler(packet, context);
		return true;
	}

	void ServerPacketHandlers::setAllowedHosts(const std::unordered_set<std::string>& hosts) {
		m_activeAllowedHosts = hosts;
	}

	void ServerPacketHandlers::registerHandler(PacketType type, const PacketHandler& handler) {
		auto rawType = utils::to_underlying_type(type);
		if (rawType >= m_descriptors.size())
			m_descriptors.resize(rawType + 1);

		if (m_descriptors[rawType].Handler)
			CATAPULT_THROW_RUNTIME_ERROR_1("handler for type is already registered", rawType);

		m_descriptors[rawType] = { handler, m_activeAllowedHosts };
	}

	const ServerPacketHandlers::PacketHandlerDescriptor* ServerPacketHandlers::findDescriptor(const Packet& packet) const {
		auto rawType = utils::to_underlying_type(packet.Type);
		if (rawType >= m_descriptors.size()) {
			CATAPULT_LOG(warning) << "requested unknown handler: " << packet;
			return nullptr;
		}

		const auto& descriptor = m_descriptors[rawType];
		return descriptor.Handler ? &descriptor : nullptr;
	}

	// endregion
}}
