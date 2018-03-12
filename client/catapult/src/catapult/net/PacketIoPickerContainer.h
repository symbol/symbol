#pragma once
#include "PacketIoPicker.h"
#include <vector>

namespace catapult { namespace net {

	/// A collection of packet io pickers.
	class PacketIoPickerContainer {
	public:
		/// Inserts \a picker, which is compatible with the specified \a roles, into this container.
		void insert(PacketIoPicker& picker, ionet::NodeRoles roles);

		/// Retrieves a packet io pair around an active connection from each picker with compatible \a roles.
		/// The connections will timeout after \a ioDuration.
		std::vector<ionet::NodePacketIoPair> pickMatching(const utils::TimeSpan& ioDuration, ionet::NodeRoles roles) const;

	private:
		std::vector<std::pair<ionet::NodeRoles, PacketIoPicker*>> m_pickers;
	};
}}
