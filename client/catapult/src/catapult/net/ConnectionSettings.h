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
#include "catapult/ionet/PacketSocketOptions.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/model/NodeIdentity.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/constants.h"

namespace catapult { namespace net {

	/// Settings used to configure connections.
	struct ConnectionSettings {
	public:
		/// Creates default settings.
		ConnectionSettings()
				: NetworkIdentifier(model::NetworkIdentifier::Zero)
				, NodeIdentityEqualityStrategy(model::NodeIdentityEqualityStrategy::Key)
				, Timeout(utils::TimeSpan::FromSeconds(10))
				, SocketWorkingBufferSize(utils::FileSize::FromKilobytes(4))
				, SocketWorkingBufferSensitivity(0) // memory reclamation disabled
				, MaxPacketDataSize(utils::FileSize::FromBytes(Default_Max_Packet_Data_Size))
				, AllowIncomingSelfConnections(true)
				, AllowOutgoingSelfConnections(false)
		{}

	public:
		/// Network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// Node identity equality strategy.
		model::NodeIdentityEqualityStrategy NodeIdentityEqualityStrategy;

		/// Connection timeout.
		utils::TimeSpan Timeout;

		/// Socket working buffer size.
		utils::FileSize SocketWorkingBufferSize;

		/// Socket working buffer sensitivity.
		size_t SocketWorkingBufferSensitivity;

		/// Maximum packet data size.
		utils::FileSize MaxPacketDataSize;

		/// Allows incoming self connections when \c true.
		bool AllowIncomingSelfConnections;

		/// Allows outgoing self connections when \c true.
		bool AllowOutgoingSelfConnections;

		/// Ssl options.
		ionet::PacketSocketSslOptions SslOptions;

	public:
		/// Gets the packet socket options represented by the configured settings.
		ionet::PacketSocketOptions toSocketOptions() const {
			ionet::PacketSocketOptions options;
			options.AcceptHandshakeTimeout = Timeout;
			options.WorkingBufferSize = SocketWorkingBufferSize.bytes();
			options.WorkingBufferSensitivity = SocketWorkingBufferSensitivity;
			options.MaxPacketDataSize = MaxPacketDataSize.bytes();
			options.SslOptions = SslOptions;
			return options;
		}
	};
}}
