#include "PublisherUtils.h"

namespace catapult { namespace zeromq {

	std::vector<uint8_t> CreateTopic(TransactionMarker marker, const Address& address) {
		std::vector<uint8_t> topic;
		topic.push_back(utils::to_underlying_type(marker));
		topic.insert(topic.end(), address.cbegin(), address.cend());
		return topic;
	}
}}
