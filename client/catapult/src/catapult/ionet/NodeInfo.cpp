#include "NodeInfo.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"
#include <algorithm>

namespace catapult { namespace ionet {

#define DEFINE_ENUM NodeSource
#define ENUM_LIST NODE_SOURCE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	namespace {
		template<typename TIterator>
		auto* FindByIdentifier(TIterator begin, TIterator end, ServiceIdentifier serviceId) {
			auto iter = std::find_if(begin, end, [serviceId](const auto& pair) {
				return serviceId == pair.first;
			});

			return end == iter ? nullptr : &iter->second;
		}
	}

	NodeInfo::NodeInfo(NodeSource source) : m_source(source)
	{}

	NodeSource NodeInfo::source() const {
		return m_source;
	}

	size_t NodeInfo::numConnectionStates() const {
		return m_connectionStates.size();
	}

	NodeInfo::ServiceIdentifiers NodeInfo::services() const {
		ServiceIdentifiers serviceIds;
		for (const auto& pair : m_connectionStates)
			serviceIds.insert(pair.first);

		return serviceIds;
	}

	bool NodeInfo::hasActiveConnection() const {
		return std::any_of(m_connectionStates.cbegin(), m_connectionStates.cend(), [](const auto& pair) {
			return 0 != pair.second.Age;
		});
	}

	const ConnectionState* NodeInfo::getConnectionState(ServiceIdentifier serviceId) const {
		return FindByIdentifier(m_connectionStates.cbegin(), m_connectionStates.cend(), serviceId);
	}

	void NodeInfo::source(NodeSource source) {
		m_source = source;
	}

	ConnectionState& NodeInfo::provisionConnectionState(ServiceIdentifier serviceId) {
		auto* pConnectionState = FindByIdentifier(m_connectionStates.begin(), m_connectionStates.end(), serviceId);
		if (pConnectionState)
			return *pConnectionState;

		m_connectionStates.emplace_back(serviceId, ConnectionState());
		return m_connectionStates.back().second;
	}

	void NodeInfo::clearAge(ServiceIdentifier serviceId) {
		auto* pConnectionState = FindByIdentifier(m_connectionStates.begin(), m_connectionStates.end(), serviceId);
		if (!pConnectionState)
			return;

		pConnectionState->Age = 0;
	}
}}
