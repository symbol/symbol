#pragma once
#include "catapult/ionet/NodePacketIoPair.h"
#include <memory>

namespace catapult { namespace utils { class TimeSpan; } }

namespace catapult { namespace net {

	/// An interface for picking packet io pairs.
	class PacketIoPicker {
	public:
		virtual ~PacketIoPicker() {}

	public:
		/// Retrieves a packet io pair around an active connection or an empty pair if no connections are available.
		/// The connection will timeout after \a ioDuration.
		virtual ionet::NodePacketIoPair pickOne(const utils::TimeSpan& ioDuration) = 0;
	};

	/// Retrieves io interfaces to at most \a numRequested connections from \a picker.
	/// The connections will timeout after \a ioDuration.
	std::vector<ionet::NodePacketIoPair> PickMultiple(PacketIoPicker& picker, size_t numRequested, const utils::TimeSpan& ioDuration);
}}
