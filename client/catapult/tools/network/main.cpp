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

#include "tools/ToolMain.h"
#include "tools/KeyValueOutputBuilder.h"
#include "tools/NetworkCensusTool.h"
#include "tools/ToolThreadUtils.h"
#include "catapult/api/RemoteNodeApi.h"
#include "catapult/extensions/RemoteDiagnosticApi.h"
#include "catapult/utils/Functional.h"
#include <algorithm>
#include <cctype>

namespace catapult { namespace tools { namespace network {

	namespace {
		// region node info

		struct NodeInfo {
		public:
			explicit NodeInfo(const ionet::Node& localNode)
					: Local(localNode)
					// always correlate partners by key because even in host based network it is guaranteed to be unique
					// also `peersInfo` api doesn't fill in NodeIdentity::Host
					, Partners(model::CreateNodeIdentityMap<ionet::Node>(model::NodeIdentityEqualityStrategy::Key))
			{}

		public:
			ionet::Node Local;
			ionet::Node Remote;
			model::NodeIdentity RemoteIdentity;
			model::NodeIdentityMap<ionet::Node> Partners;
			model::EntityRange<ionet::PackedNodeInfo> PartnerNodeInfos;
		};

		using NodeInfoPointer = NetworkCensusTool<NodeInfo>::NodeInfoPointer;

		// endregion

		// region formatting

		using IdentityNameMap = model::NodeIdentityMap<std::string>;

		std::vector<std::string> GetComponentStrings(ionet::NodeRoles roles) {
			std::vector<std::string> parts;
			if (HasFlag(ionet::NodeRoles::Peer, roles))
				parts.push_back("Peer");

			if (HasFlag(ionet::NodeRoles::Api, roles))
				parts.push_back("Api");

			return parts;
		}

		class SummaryBuilder : public KeyValueOutputBuilder {
		public:
			using KeyValueOutputBuilder::add;

			void add(const std::string& key, ionet::NodeVersion version) {
				auto rawVersion = version.unwrap();
				std::vector<std::string> parts(sizeof(ionet::NodeVersion));
				for (auto i = 0u; i < parts.size(); ++i) {
					parts[parts.size() - 1 - i] = std::to_string(rawVersion & 0xFF);
					rawVersion >>= 8;
				}

				add(key, parts, ".");
			}

			void add(const std::string& key, ionet::NodeRoles roles) {
				add(key, GetComponentStrings(roles), ", ");
			}
		};

		std::string Summarize(const ionet::Node& node, const model::NodeIdentity& nodeIdentity) {
			SummaryBuilder builder;
			builder.add("PublicKey (Identity)", nodeIdentity.PublicKey);
			builder.add("Host (Identity)", nodeIdentity.Host);

			builder.add("IdentityKey", node.identity().PublicKey);
			builder.add("Host", node.endpoint().Host);
			builder.add("Port", node.endpoint().Port);
			builder.add("Network", node.metadata().NetworkFingerprint);
			builder.add("Name", node.metadata().Name);
			builder.add("Version", node.metadata().Version);
			builder.add("Roles", node.metadata().Roles);
			return builder.str(" < ");
		}

		std::string GetName(const IdentityNameMap& identityNameMap, const ionet::Node& node) {
			auto iter = identityNameMap.find(node.identity());
			return identityNameMap.cend() != iter
					? iter->second
					: !node.metadata().Name.empty() ? node.metadata().Name : "?";
		}

		const ionet::PackedNodeInfo* TryFindPartnerNodeInfo(
				const model::NodeIdentity& partnerIdentity,
				const model::EntityRange<ionet::PackedNodeInfo>& partnerNodeInfos) {
			auto iter = std::find_if(partnerNodeInfos.cbegin(), partnerNodeInfos.cend(), [&partnerIdentity](const auto& nodeInfo) {
				return partnerIdentity.PublicKey == nodeInfo.IdentityKey;
			});

			return partnerNodeInfos.cend() == iter ? nullptr : &*iter;
		}

		std::string ToString(ionet::NodeSource source) {
			std::ostringstream out;
			out << source;
			return out.str();
		}

		std::string ToString(ionet::ServiceIdentifier serviceId) {
			// service id is four ASCII characters by convention, so decode
			std::string str;
			auto value = serviceId.unwrap();
			while (0 != value) {
				auto ch = static_cast<char>(value & 0xFF);
				str.insert(str.begin(), std::isalnum(ch) ? ch : '?');
				value >>= 8;
			}

			std::ostringstream out;
			out << str << " (" << utils::HexFormat(serviceId) << ")";
			return out.str();
		}

		std::string ToString(const ionet::PackedNodeInteractions& interactions) {
			std::ostringstream out;
			out
					<< "successes: " << interactions.NumSuccesses
					<< ", failures: " << interactions.NumFailures;
			return out.str();
		}

		std::string ToString(const ionet::PackedConnectionState& connectionState) {
			std::ostringstream out;
			out
					<< "{ "
					<< "age: " << connectionState.Age
					<< ", ban-age: " << connectionState.BanAge
					<< ", c-failures: " << connectionState.NumConsecutiveFailures
					<< " }";
			return out.str();
		}

		std::string SummarizePartners(const NodeInfo& nodeInfo, const IdentityNameMap& identityNameMap) {
			// sort nodes by role and host
			std::vector<ionet::Node> nodes;
			for (const auto& pair : nodeInfo.Partners)
				nodes.push_back(pair.second);

			std::sort(nodes.begin(), nodes.end(), [](const auto& lhs, const auto& rhs) {
				if (lhs.metadata().Roles != rhs.metadata().Roles)
					return lhs.metadata().Roles < rhs.metadata().Roles;

				return lhs.endpoint().Host < rhs.endpoint().Host;
			});

			SummaryBuilder builder;
			builder.add("COUNT", nodes.size());
			for (const auto& partnerNode : nodes) {
				builder.add("---", "---");

				// output partner host and roles
				builder.add(partnerNode.endpoint().Host, partnerNode.identity().PublicKey);
				builder.add(GetName(identityNameMap, partnerNode), partnerNode.metadata().Roles);

				// get detailed partner information
				const auto* pPartnerNodeInfo = TryFindPartnerNodeInfo(partnerNode.identity(), nodeInfo.PartnerNodeInfos);
				if (!pPartnerNodeInfo)
					continue;

				// output interactions
				builder.add("interactions", ToString(pPartnerNodeInfo->Interactions));

				// output source and number of connections
				builder.add(ToString(pPartnerNodeInfo->Source), static_cast<uint16_t>(pPartnerNodeInfo->ConnectionStatesCount));

				// output detailed per-connection information
				const auto* pConnectionState = pPartnerNodeInfo->ConnectionStatesPtr();
				for (auto i = 0u; i < pPartnerNodeInfo->ConnectionStatesCount; ++i, ++pConnectionState)
					builder.add(ToString(pConnectionState->ServiceId), ToString(*pConnectionState));
			}

			return builder.str(" --- ");
		}

		class PeersTableGenerator {
		public:
			PeersTableGenerator(const std::vector<NodeInfoPointer>& nodeInfos, size_t cellWidth)
					: m_nodeInfos(nodeInfos)
					, m_cellWidth(cellWidth)
			{}

		public:
			std::string str() {
				printHeader();
				m_out << std::endl;

				printHorizontalSeparator();
				m_out << std::endl;

				for (const auto& pNodeInfo : m_nodeInfos) {
					printRow(*pNodeInfo);
					m_out << std::endl;
				}

				printHorizontalSeparator();
				return m_out.str();
			}

		private:
			void printHeader() {
				printCellValue("");
				for (const auto& pNodeInfo : m_nodeInfos)
					printCellValue(toHeaderString(*pNodeInfo));
			}

			void printHorizontalSeparator() {
				for (auto i = 0u; i <= m_nodeInfos.size(); ++i)
					printCellValue(std::string(m_cellWidth, '-'));
			}

			void printRow(const NodeInfo& rowNodeInfo) {
				printCellValue(toHeaderString(rowNodeInfo));
				for (const auto& pNodeInfo : m_nodeInfos) {
					auto partnerIter = rowNodeInfo.Partners.find(pNodeInfo->Local.identity());
					printCellValue(pNodeInfo->Partners.cend() != partnerIter ? toValueString(rowNodeInfo, partnerIter->second) : "");
				}
			}

			void printCellValue(const std::string& value) {
				m_out << " " << std::setw(static_cast<int>(m_cellWidth)) << value << " |";
			}

			const std::string& toHeaderString(const NodeInfo& nodeInfo) {
				return nodeInfo.Local.metadata().Name;
			}

			std::string toValueString(const NodeInfo& rowNodeInfo, const ionet::Node& columnNode) {
				const auto* pPartnerNodeInfo = TryFindPartnerNodeInfo(columnNode.identity(), rowNodeInfo.PartnerNodeInfos);
				if (!pPartnerNodeInfo)
					return "ERROR";

				// find all connections between row and column nodes
				// as connection id, use first character of decoded service id; uppercase if active, lowercase if not
				std::vector<char> connectionIds;
				const auto* pConnectionState = pPartnerNodeInfo->ConnectionStatesPtr();
				for (auto i = 0u; i < pPartnerNodeInfo->ConnectionStatesCount; ++i, ++pConnectionState) {
					auto connectionId = ToString(pConnectionState->ServiceId)[0];
					if (0 == pConnectionState->Age)
						connectionId = static_cast<char>(std::tolower(connectionId));

					connectionIds.push_back(connectionId);
				}

				// cell value should be result of sorting and joining all connection ids
				std::sort(connectionIds.begin(), connectionIds.end());
				return Join(connectionIds, " ");
			}

		private:
			const std::vector<NodeInfoPointer>& m_nodeInfos;
			size_t m_cellWidth;
			std::ostringstream m_out;
		};

		void PrettyPrint(const std::vector<NodeInfoPointer>& nodeInfos) {
			// 0. create an identity to name map based on local information (static nodes might not broadcast their names)
			IdentityNameMap identityNameMap(model::CreateNodeIdentityMap<std::string>(model::NodeIdentityEqualityStrategy::Key_And_Host));

			size_t maxNameSize = 0;
			for (const auto& pNodeInfo : nodeInfos) {
				const auto& node = pNodeInfo->Local;
				identityNameMap.emplace(node.identity(), node.metadata().Name);
				maxNameSize = std::max(maxNameSize, node.metadata().Name.size());
			}

			// 1. output detailed node information reported by each node about itself
			CATAPULT_LOG(info) << "--- NODE INFO for known peers ---";
			for (const auto& pNodeInfo : nodeInfos)
				CATAPULT_LOG(info) << "> " << pNodeInfo->Local << Summarize(pNodeInfo->Remote, pNodeInfo->RemoteIdentity);

			// 2. output summary information of peers reported by each node
			CATAPULT_LOG(info) << "--- PARTNER INFO for known peers ---";
			for (const auto& pNodeInfo : nodeInfos)
				CATAPULT_LOG(info) << "> " << pNodeInfo->Local << SummarizePartners(*pNodeInfo, identityNameMap);

			// 3. output table (only for static nodes)
			PeersTableGenerator generator(nodeInfos, maxNameSize);
			CATAPULT_LOG(info) << "--- PARTNER INFO TABLE for known peers ---";
			CATAPULT_LOG(info) << std::endl << generator.str();
		}

		// endregion

		class NetworkTool : public NetworkCensusTool<NodeInfo> {
		public:
			NetworkTool() : NetworkCensusTool("Network")
			{}

		private:
			std::vector<thread::future<bool>> getNodeInfoFutures(
					thread::IoThreadPool&,
					ionet::PacketIo& io,
					const model::NodeIdentity& nodeIdentity,
					NodeInfo& nodeInfo) override {
				auto pApi = api::CreateRemoteNodeApi(io);
				auto pDiagnosticApi = extensions::CreateRemoteDiagnosticApi(io);

				std::vector<thread::future<bool>> infoFutures;
				infoFutures.emplace_back(pApi->nodeInfo().then([nodeIdentity, &nodeInfo](auto&& nodeFuture) {
					return UnwrapFutureAndSuppressErrors("querying node info", std::move(nodeFuture), [nodeIdentity, &nodeInfo](
							const auto& remoteNode) {
						nodeInfo.RemoteIdentity = nodeIdentity;
						nodeInfo.Remote = remoteNode;
					});
				}));
				infoFutures.emplace_back(pApi->peersInfo().then([&nodeInfo](auto&& peersFuture) {
					return UnwrapFutureAndSuppressErrors("querying peers info", std::move(peersFuture), [&nodeInfo](const auto& nodes) {
						for (const auto& node : nodes)
							nodeInfo.Partners.emplace(node.identity(), node);
					});
				}));
				infoFutures.emplace_back(pDiagnosticApi->activeNodeInfos().then([&nodeInfo](auto&& nodeInfosFuture) {
					auto message = "querying peers diagnostics";
					return UnwrapFutureAndSuppressErrors(message, std::move(nodeInfosFuture), [&nodeInfo](auto&& nodeInfos) {
						nodeInfo.PartnerNodeInfos = std::move(nodeInfos);
					});
				}));

				return infoFutures;
			}

			size_t processNodeInfos(const std::vector<NodeInfoPointer>& nodeInfos) override {
				PrettyPrint(nodeInfos);

				return utils::Sum(nodeInfos, [](const auto& pNodeInfo) {
					return Key() == pNodeInfo->Remote.identity().PublicKey ? 1u : 0;
				});
			}
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::network::NetworkTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
