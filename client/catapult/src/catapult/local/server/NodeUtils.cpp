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

#include "NodeUtils.h"
#include "catapult/config/CatapultConfiguration.h"
#include "catapult/ionet/NodeContainer.h"

namespace catapult { namespace local {

	// region ValidateNodes / AddLocalNode

	namespace {
		void CheckString(const std::string& str, const char* name) {
			if (str.size() <= std::numeric_limits<uint8_t>::max())
				return;

			std::ostringstream out;
			out << name << " is too long (" << str << ")";
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		void ValidateNode(const ionet::Node& node) {
			CheckString(node.endpoint().Host, "host");
			CheckString(node.metadata().Name, "name");
		}
	}

	void ValidateNodes(const std::vector<ionet::Node>& nodes) {
		for (const auto& node : nodes)
			ValidateNode(node);
	}

	void AddLocalNode(ionet::NodeContainer& nodes, const config::CatapultConfiguration& config) {
		auto localNode = config::ToLocalNode(config);
		ValidateNode(localNode);
		nodes.modifier().add(localNode, ionet::NodeSource::Local);
	}

	// endregion

	// region GetBanSettings

	ionet::BanSettings GetBanSettings(const config::NodeConfiguration::BanningSubConfiguration& banConfig) {
		ionet::BanSettings banSettings;
		banSettings.DefaultBanDuration = banConfig.DefaultBanDuration;
		banSettings.MaxBanDuration = banConfig.MaxBanDuration;
		banSettings.KeepAliveDuration = banConfig.KeepAliveDuration;
		banSettings.MaxBannedNodes = banConfig.MaxBannedNodes;
		return banSettings;
	}

	// endregion
}}
