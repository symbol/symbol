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
#include "IoTypes.h"
#include "PacketPayload.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/constants.h"
#include "catapult/functions.h"
#include "catapult/types.h"
#include <unordered_set>
#include <vector>

namespace catapult { namespace ionet {

	/// Context passed to a server packet handler function.
	struct ServerPacketHandlerContext : utils::MoveOnly {
	public:
		/// Creates an empty context.
		ServerPacketHandlerContext();

		/// Creates a context around \a key and \ host.
		ServerPacketHandlerContext(const Key& key, const std::string& host);

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
		/// Sets the response associated with this context to \a payload.
		void response(PacketPayload&& payload);

	private:
		const Key* m_pKey;
		const std::string* m_pHost;
		bool m_hasResponse;
		PacketPayload m_payload;

		Key m_defaultKey;
		std::string m_defaultHost;
	};

	/// Collection of packet handlers where there is at most one handler per packet type.
	class ServerPacketHandlers {
	public:
		/// Handler context type.
		using ContextType = ServerPacketHandlerContext;

		/// Packet handler function.
		using PacketHandler = consumer<const Packet&, ContextType&>;

	public:
		/// Creates packet handlers with a max packet data size (\a maxPacketDataSize).
		explicit ServerPacketHandlers(uint32_t maxPacketDataSize = Default_Max_Packet_Data_Size);

	public:
		/// Gets the number of registered handlers.
		size_t size() const;

		/// Gets the max packet data size.
		uint32_t maxPacketDataSize() const;

		/// Determines if \a type can be processed by a registered handler.
		bool canProcess(PacketType type) const;

		/// Determines if \a packet can be processed by a registered handler.
		bool canProcess(const Packet& packet) const;

		/// Processes \a packet using the specified \a context and returns \c true if the
		/// packet was processed.
		bool process(const Packet& packet, ContextType& context) const;

	public:
		/// Sets the \a hosts that are allowed to access subsequently registered handlers.
		void setAllowedHosts(const std::unordered_set<std::string>& hosts);

		/// Registers \a handler for the specified packet \a type.
		void registerHandler(PacketType type, const PacketHandler& handler);

	private:
		struct PacketHandlerDescriptor {
			PacketHandler Handler;
			std::unordered_set<std::string> AllowedHosts;
		};

	private:
		const PacketHandlerDescriptor* findDescriptor(const Packet& packet) const;

	private:
		uint32_t m_maxPacketDataSize;
		std::vector<PacketHandlerDescriptor> m_descriptors;
		std::unordered_set<std::string> m_activeAllowedHosts;
	};
}}
