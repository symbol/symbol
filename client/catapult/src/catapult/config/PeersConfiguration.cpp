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

#include "PeersConfiguration.h"
#include "catapult/ionet/Node.h"
#include "catapult/utils/HexParser.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"

#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif

#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

namespace catapult { namespace config {

	namespace {
		template<typename T>
		auto GetOptional(const pt::ptree& tree, const std::string& key) {
			auto value = tree.get_optional<T>(key);
			return value.is_initialized() ? value.get() : T();
		}

		template<typename T>
		auto Get(const pt::ptree& tree, const std::string& key) {
			// use get_optional instead of get in order to allow better error messages to propagate out
			auto value = tree.get_optional<T>(key);
			if (!value.is_initialized()) {
				std::ostringstream message;
				message << "required property '" << key << "' was not found in json";
				CATAPULT_THROW_RUNTIME_ERROR(message.str().c_str());
			}

			return value.get();
		}

		auto GetChild(const pt::ptree& tree, const std::string& key) {
			// use get_child_optional instead of get_child in order to allow better error messages to propagate out
			auto value = tree.get_child_optional(key);
			if (!value.is_initialized()) {
				std::ostringstream message;
				message << "required child '" << key << "' was not found in json";
				CATAPULT_THROW_RUNTIME_ERROR(message.str().c_str());
			}

			return value.get();
		}

		ionet::NodeRoles ParseRoles(const std::string& str) {
			ionet::NodeRoles roles;
			if (!ionet::TryParseValue(str, roles)) {
				std::ostringstream message;
				message << "roles property has unsupported value: " << str;
				CATAPULT_THROW_RUNTIME_ERROR(message.str().c_str());
			}

			return roles;
		}

		std::vector<ionet::Node> LoadPeersFromProperties(
				const pt::ptree& properties,
				const model::UniqueNetworkFingerprint& networkFingerprint) {
			if (!GetOptional<std::string>(properties, "knownPeers").empty())
				CATAPULT_THROW_RUNTIME_ERROR("knownPeers must be an array");

			std::vector<ionet::Node> peers;
			for (const auto& peerJson : GetChild(properties, "knownPeers")) {
				const auto& endpointJson = GetChild(peerJson.second, "endpoint");
				const auto& metadataJson = GetChild(peerJson.second, "metadata");

				auto identityKey = utils::ParseByteArray<Key>(Get<std::string>(peerJson.second, "publicKey"));
				auto endpoint = ionet::NodeEndpoint{ Get<std::string>(endpointJson, "host"), Get<unsigned short>(endpointJson, "port") };
				auto metadata = ionet::NodeMetadata(networkFingerprint, GetOptional<std::string>(metadataJson, "name"));
				metadata.Roles = ParseRoles(Get<std::string>(metadataJson, "roles"));
				peers.push_back({ { identityKey, endpoint.Host }, endpoint, metadata });
			}

			return peers;
		}
	}

	std::vector<ionet::Node> LoadPeersFromStream(std::istream& input, const model::UniqueNetworkFingerprint& networkFingerprint) {
		pt::ptree properties;
		pt::read_json(input, properties);
		return LoadPeersFromProperties(properties, networkFingerprint);
	}

	std::vector<ionet::Node> LoadPeersFromPath(const std::string& path, const model::UniqueNetworkFingerprint& networkFingerprint) {
		std::ifstream inputStream(path);
		return LoadPeersFromStream(inputStream, networkFingerprint);
	}
}}
