#include "PeersConfiguration.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/ionet/Node.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"

#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>

#pragma warning(push)
#pragma warning(disable:4715) /* "not all control paths return a value" */
#endif
#include <boost/property_tree/json_parser.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

		std::vector<ionet::Node> LoadPeersFromProperties(const pt::ptree& properties, model::NetworkIdentifier networkIdentifier) {
			if (!GetOptional<std::string>(properties, "knownPeers").empty())
				CATAPULT_THROW_RUNTIME_ERROR("knownPeers must be an array");

			std::vector<ionet::Node> peers;
			for (const auto& peerJson : GetChild(properties, "knownPeers")) {
				const auto& endpointJson = GetChild(peerJson.second, "endpoint");
				const auto& metadataJson = GetChild(peerJson.second, "metadata");

				auto identityKey = crypto::ParseKey(Get<std::string>(peerJson.second, "publicKey"));
				auto endpoint = ionet::NodeEndpoint{ Get<std::string>(endpointJson, "host"), Get<unsigned short>(endpointJson, "port") };
				auto metadata = ionet::NodeMetadata(networkIdentifier, GetOptional<std::string>(metadataJson, "name"));
				metadata.Roles = ParseRoles(Get<std::string>(metadataJson, "roles"));
				peers.push_back({ identityKey, endpoint, metadata });
			}

			return peers;
		}
	}

	std::vector<ionet::Node> LoadPeersFromStream(std::istream& input, model::NetworkIdentifier networkIdentifier) {
		pt::ptree properties;
		pt::read_json(input, properties);
		return LoadPeersFromProperties(properties, networkIdentifier);
	}

	std::vector<ionet::Node> LoadPeersFromPath(const std::string& path, model::NetworkIdentifier networkIdentifier) {
		std::ifstream inputStream(path);
		return LoadPeersFromStream(inputStream, networkIdentifier);
	}
}}
