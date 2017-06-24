#pragma once
#include "IoTypes.h"
#include "Packet.h"
#include "PacketPayload.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/exceptions.h"
#include <functional>
#include <vector>

namespace catapult { namespace ionet {

	/// Context passed to a server packet handler function.
	struct ServerPacketHandlerContext : utils::MoveOnly {
	public:
		/// Returns \c true if a response has been associated with this context.
		bool hasResponse() const {
			return !m_payload.unset();
		}

		/// Gets the response associated with this context.
		const PacketPayload& response() const {
			if (!hasResponse())
				CATAPULT_THROW_RUNTIME_ERROR("response is not set");

			return m_payload;
		}

	public:
		/// Sets \a payload as the response associated with this context.
		void response(PacketPayload&& payload) {
			if (hasResponse())
				CATAPULT_THROW_RUNTIME_ERROR("response is already set");

			m_payload = std::move(payload);
		}

	private:
		PacketPayload m_payload;
	};

	/// A collection of packet handlers where there is at most one handler per packet type.
	class ServerPacketHandlers {
	public:
		/// The handler context type.
		using ContextType = ServerPacketHandlerContext;

		/// Packet handler function.
		using PacketHandler = std::function<void (const Packet&, ContextType&)>;

	public:
		/// Gets the number of registered handlers.
		size_t size() const {
			size_t numHandlers = 0;
			for (const auto& handler : m_handlers) numHandlers += handler ? 1 : 0;
			return numHandlers;
		}

		/// Determines if \a packet can be processed by a registered handler.
		bool canProcess(const Packet& packet) const {
			return nullptr != findHandler(packet);
		}

		/// Processes \a packet using the specified \a context and returns \c true if the
		/// packet was processed.
		bool process(const Packet& packet, ContextType& context) const {
			auto pHandler = findHandler(packet);
			if (!pHandler) return false;

			(*pHandler)(packet, context);
			return true;
		}

	public:
		/// Registers a \a handler for the specified packet \a type.
		void registerHandler(PacketType type, const PacketHandler& handler) {
			auto rawType = utils::to_underlying_type(type);
			if (rawType >= m_handlers.size())
				m_handlers.resize(rawType + 1);

			if (m_handlers[rawType])
				CATAPULT_THROW_RUNTIME_ERROR_1("handler for type is already registered", rawType);

			m_handlers[rawType] = handler;
		}

	private:
		const PacketHandler* findHandler(const Packet& packet) const {
			auto rawType = utils::to_underlying_type(packet.Type);
			if (rawType >= m_handlers.size()) {
				CATAPULT_LOG(warning) << "requested handler of unknown type " << rawType;
				return nullptr;
			}

			const auto& handler = m_handlers[rawType];
			return handler ? &handler : nullptr;
		}

	private:
		std::vector<PacketHandler> m_handlers;
	};
}}
