#pragma once
#include "IoTypes.h"
#include "PacketPayload.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/functions.h"
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace ionet {

	/// Context passed to a server packet handler function.
	struct ServerPacketHandlerContext : utils::MoveOnly {
	public:
		/// Creates a context around \a key and \ host.
		explicit ServerPacketHandlerContext(const Key& key, const std::string& host);

	public:
		/// Gets the public key associated with the client.
		const Key& key() const;

		/// Gets the host associated with the client.
		const std::string& host() const;

		/// Returns \c true if a response has been associated with this context.
		bool hasResponse() const;

		/// Gets the response associated with this context.
		const PacketPayload& response() const;

	public:
		/// Sets \a payload as the response associated with this context.
		void response(PacketPayload&& payload);

	private:
		const Key& m_key;
		const std::string& m_host;
		PacketPayload m_payload;
	};

	/// A collection of packet handlers where there is at most one handler per packet type.
	class ServerPacketHandlers {
	public:
		/// The handler context type.
		using ContextType = ServerPacketHandlerContext;

		/// Packet handler function.
		using PacketHandler = consumer<const Packet&, ContextType&>;

	public:
		/// Gets the number of registered handlers.
		size_t size() const;

		/// Determines if \a type can be processed by a registered handler.
		bool canProcess(PacketType type) const;

		/// Determines if \a packet can be processed by a registered handler.
		bool canProcess(const Packet& packet) const;

		/// Processes \a packet using the specified \a context and returns \c true if the
		/// packet was processed.
		bool process(const Packet& packet, ContextType& context) const;

	public:
		/// Registers a \a handler for the specified packet \a type.
		void registerHandler(PacketType type, const PacketHandler& handler);

	private:
		const PacketHandler* findHandler(const Packet& packet) const ;

	private:
		std::vector<PacketHandler> m_handlers;
	};
}}
