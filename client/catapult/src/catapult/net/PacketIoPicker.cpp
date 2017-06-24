#include "PacketIoPicker.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace net {

	std::vector<ionet::NodePacketIoPair> PickMultiple(
			PacketIoPicker& picker,
			size_t numRequested,
			const utils::TimeSpan& ioDuration) {
		std::vector<ionet::NodePacketIoPair> pairs;
		for (auto i = 0u; i < numRequested; ++i) {
			auto packetIoPair = picker.pickOne(ioDuration);
			if (!packetIoPair)
				break;

			pairs.push_back(packetIoPair);
		}

		return pairs;
	}
}}
