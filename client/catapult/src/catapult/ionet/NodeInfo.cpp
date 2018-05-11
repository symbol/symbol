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
