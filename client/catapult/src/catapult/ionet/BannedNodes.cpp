/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "BannedNodes.h"
#include "catapult/utils/ContainerHelpers.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace ionet {

	BannedNodes::BannedNodes(
			const BanSettings& banSettings,
			const supplier<Timestamp>& timeSupplier,
			model::NodeIdentityEqualityStrategy equalityStrategy)
			: m_banSettings(banSettings)
			, m_timeSupplier(timeSupplier)
			, m_bannedNodes(model::CreateNodeIdentityMap<BannedNode>(equalityStrategy))
	{}

	size_t BannedNodes::size() const {
		auto size = std::count_if(m_bannedNodes.cbegin(), m_bannedNodes.cend(), [timestamp = m_timeSupplier()](const auto& pair) {
			return IsBanned(pair.second, timestamp);
		});

		return static_cast<size_t>(size);
	}

	size_t BannedNodes::deepSize() const {
		return m_bannedNodes.size();
	}

	bool BannedNodes::isBanned(const model::NodeIdentity& nodeIdentity) const {
		auto iter = m_bannedNodes.find(nodeIdentity);
		if (m_bannedNodes.cend() == iter)
			return false;

		const auto& bannedNode = iter->second;
		return IsBanned(bannedNode, m_timeSupplier());
	}

	void BannedNodes::add(const model::NodeIdentity& nodeIdentity, uint32_t reason) {
		if ("_local_" == nodeIdentity.Host)
			return;

		auto defaultBanDuration = m_banSettings.DefaultBanDuration;
		auto iter = m_bannedNodes.find(nodeIdentity);
		auto timestamp = m_timeSupplier();
		if (m_bannedNodes.cend() == iter) {
			if (m_banSettings.MaxBannedNodes <= m_bannedNodes.size())
				removeEntryThatExpiresNext();

			BannedNode bannedNode{ nodeIdentity, timestamp, defaultBanDuration, reason };
			iter = m_bannedNodes.emplace(nodeIdentity, bannedNode).first;
		} else {
			auto& bannedNode = iter->second;
			bannedNode.BanStart = timestamp;
			auto newBanTimeSpan = utils::TimeSpan::FromMilliseconds(bannedNode.BanDuration.millis() + defaultBanDuration.millis());
			bannedNode.BanDuration = std::min<utils::TimeSpan>(newBanTimeSpan, m_banSettings.MaxBanDuration);
		}

		CATAPULT_LOG(warning)
				<< "banning node with identity " << nodeIdentity
				<< " for " << iter->second.BanDuration
				<< ", reason: " << utils::HexFormat(iter->second.Reason);
	}

	void BannedNodes::prune() {
		auto keepAliveMillis = m_banSettings.KeepAliveDuration.millis();
		auto timestamp = m_timeSupplier();
		utils::map_erase_if(m_bannedNodes, [timestamp, keepAliveMillis](const auto& pair) {
			auto timeSpan = utils::TimeSpan::FromMilliseconds(pair.second.BanDuration.millis() + keepAliveMillis);
			return pair.second.BanStart + timeSpan <= timestamp;
		});
	}

	void BannedNodes::removeEntryThatExpiresNext() {
		auto iter = std::min_element(m_bannedNodes.cbegin(), m_bannedNodes.cend(), [](const auto& pair1, const auto& pair2) {
			return pair1.second.BanStart + pair1.second.BanDuration < pair2.second.BanStart + pair2.second.BanDuration;
		});
		m_bannedNodes.erase(iter);
	}

	bool BannedNodes::IsBanned(const BannedNode& bannedNode, Timestamp timestamp) {
		return bannedNode.BanStart + bannedNode.BanDuration > timestamp;
	}
}}
