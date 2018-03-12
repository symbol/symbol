#include "PacketHandlers.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace ionet {

	// region ServerPacketHandlerContext

	ServerPacketHandlerContext::ServerPacketHandlerContext(const Key& key, const std::string& host)
			: m_key(key)
			, m_host(host)
	{}

	const Key& ServerPacketHandlerContext::key() const {
		return m_key;
	}

	const std::string& ServerPacketHandlerContext::host() const {
		return m_host;
	}

	bool ServerPacketHandlerContext::hasResponse() const {
		return !m_payload.unset();
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
	}

	// endregion

	// region ServerPacketHandlers

	size_t ServerPacketHandlers::size() const {
		size_t numHandlers = 0;
		for (const auto& handler : m_handlers)
			numHandlers += handler ? 1 : 0;

		return numHandlers;
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

		CATAPULT_LOG(trace) << "processing packet with type " << packet.Type;
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
			CATAPULT_LOG(warning) << "requested handler of unknown type " << packet.Type;
			return nullptr;
		}

		const auto& handler = m_handlers[rawType];
		return handler ? &handler : nullptr;
	}

	// endregion
}}
