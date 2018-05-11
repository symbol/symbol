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

#include "PacketHandlers.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace ionet {

	// region ServerPacketHandlerContext

	ServerPacketHandlerContext::ServerPacketHandlerContext(const Key& key, const std::string& host)
			: m_key(key)
			, m_host(host)
			, m_hasResponse(false)
	{}

	const Key& ServerPacketHandlerContext::key() const {
		return m_key;
	}

	const std::string& ServerPacketHandlerContext::host() const {
		return m_host;
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
		for (const auto& handler : m_handlers)
			numHandlers += handler ? 1 : 0;

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
		return !!findHandler(packet);
	}

	bool ServerPacketHandlers::process(const Packet& packet, ContextType& context) const {
		const auto* pHandler = findHandler(packet);
		if (!pHandler)
			return false;

		CATAPULT_LOG(trace) << "processing " << packet;
		(*pHandler)(packet, context);
		return true;
	}

	void ServerPacketHandlers::registerHandler(PacketType type, const PacketHandler& handler) {
		auto rawType = utils::to_underlying_type(type);
		if (rawType >= m_handlers.size())
			m_handlers.resize(rawType + 1);

		if (m_handlers[rawType])
			CATAPULT_THROW_RUNTIME_ERROR_1("handler for type is already registered", rawType);

		m_handlers[rawType] = handler;
	}

	const ServerPacketHandlers::PacketHandler* ServerPacketHandlers::findHandler(const Packet& packet) const {
		auto rawType = utils::to_underlying_type(packet.Type);
		if (rawType >= m_handlers.size()) {
			CATAPULT_LOG(warning) << "requested unknown handler: " << packet;
			return nullptr;
		}

		const auto& handler = m_handlers[rawType];
		return handler ? &handler : nullptr;
	}

	// endregion
}}
