#include "PeersConfiguration.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/ionet/Node.h"
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
		T GetOptional(const pt::ptree& tree, const std::string& key) {
			auto value = tree.get_optional<T>(key);
			return value.is_initialized() ? value.get() : T();
		}

		std::vector<ionet::Node> LoadPeersFromProperties(const pt::ptree& properties, model::NetworkIdentifier networkIdentifier) {
			std::vector<ionet::Node> peers;
			for (const auto& peerJson : properties.get_child("knownPeers")) {
				const auto& endpointJson = peerJson.second.get_child("endpoint");
				const auto& identityJson = peerJson.second.get_child("identity");

				auto endpoint = ionet::NodeEndpoint{
					endpointJson.get<std::string>("host"),
					endpointJson.get<unsigned short>("port")
				};

				auto identity = ionet::NodeIdentity{
					crypto::ParseKey(identityJson.get<std::string>("public-key")),
					GetOptional<std::string>(identityJson, "name")
				};
				peers.push_back({ endpoint, identity, networkIdentifier });
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
