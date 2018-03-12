#pragma once
#include "catapult/ionet/PacketSocketOptions.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace net {

	/// Settings used to configure connections.
	struct ConnectionSettings {
	public:
		/// Creates default settings.
		ConnectionSettings()
				: NetworkIdentifier(model::NetworkIdentifier::Zero)
				, Timeout(utils::TimeSpan::FromSeconds(10))
				, SocketWorkingBufferSize(utils::FileSize::FromKilobytes(4))
				, SocketWorkingBufferSensitivity(0) // memory reclamation disabled
				, MaxPacketDataSize(utils::FileSize::FromMegabytes(100))
		{}

	public:
		/// The network identifier.
		model::NetworkIdentifier NetworkIdentifier;

		/// The connection timeout.
		utils::TimeSpan Timeout;

		/// The socket working buffer size.
		utils::FileSize SocketWorkingBufferSize;

		/// The socket working buffer sensitivity.
		size_t SocketWorkingBufferSensitivity;

		/// The maximum packet data size.
		utils::FileSize MaxPacketDataSize;

	public:
		/// Gets the packet socket options represented by the configured settings.
		ionet::PacketSocketOptions toSocketOptions() const {
			ionet::PacketSocketOptions options;
			options.WorkingBufferSize = SocketWorkingBufferSize.bytes();
			options.WorkingBufferSensitivity = SocketWorkingBufferSensitivity;
			options.MaxPacketDataSize = MaxPacketDataSize.bytes();
			return options;
		}
	};
}}
