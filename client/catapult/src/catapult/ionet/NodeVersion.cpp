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

#include "NodeVersion.h"
#include "catapult/utils/ConfigurationValueParsers.h"
#include "catapult/version/version.h"

namespace catapult { namespace ionet {

	namespace {
		static constexpr auto Num_Version_Parts = 4u;

		NodeVersion PackVersion(const std::array<uint8_t, Num_Version_Parts>& versionParts) {
			NodeVersion::ValueType version = 0;
			for (auto part : versionParts) {
				version <<= 8;
				version |= part;
			}

			return NodeVersion(version);
		}
	}

	NodeVersion GetCurrentServerVersion() {
		return PackVersion({ CATAPULT_VERSION_MAJOR, CATAPULT_VERSION_MINOR, CATAPULT_VERSION_REVISION, CATAPULT_VERSION_BUILD });
	}

	bool TryParseValue(const std::string& str, NodeVersion& version) {
		if (str.empty()) {
			version = GetCurrentServerVersion();
			return true;
		}

		size_t searchIndex = 0;
		std::vector<std::string> versionParts;
		while (true) {
			auto separatorIndex = str.find('.', searchIndex);
			auto item = std::string::npos == separatorIndex
					? str.substr(searchIndex)
					: str.substr(searchIndex, separatorIndex - searchIndex);

			// don't allow empty values
			if (item.empty())
				return false;

			versionParts.push_back(item);
			if (std::string::npos == separatorIndex)
				break;

			searchIndex = separatorIndex + 1;
		}

		if (Num_Version_Parts != versionParts.size())
			return false;

		std::array<uint8_t, Num_Version_Parts> parsedVersionParts;
		for (auto i = 0u; i < Num_Version_Parts; ++i) {
			if (!utils::TryParseValue(versionParts[i], parsedVersionParts[i]))
				return false;
		}

		version = PackVersion(parsedVersionParts);
		return true;
	}
}}
