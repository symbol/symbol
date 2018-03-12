#include "PacketIoPickerContainer.h"
#include "catapult/utils/TimeSpan.h"

namespace catapult { namespace net {

	void PacketIoPickerContainer::insert(PacketIoPicker& picker, ionet::NodeRoles roles) {
		m_pickers.emplace_back(roles, &picker);
	}

	std::vector<ionet::NodePacketIoPair> PacketIoPickerContainer::pickMatching(
			const utils::TimeSpan& ioDuration,
			ionet::NodeRoles roles) const {
		std::vector<ionet::NodePacketIoPair> ioPairs;
		for (const auto& pickerPair : m_pickers) {
			if (!HasFlag(roles, pickerPair.first))
				continue;

			auto ioPair = pickerPair.second->pickOne(ioDuration);
			if (!ioPair)
				continue;

			ioPairs.push_back(ioPair);
		}

		return ioPairs;
	}
}}
