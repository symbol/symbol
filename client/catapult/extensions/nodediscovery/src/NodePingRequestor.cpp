#include "NodePingRequestor.h"
#include "NodePingUtils.h"
#include "nodediscovery/src/api/RemoteNodeApi.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/PeerConnectResult.h"
#include "catapult/net/ServerConnector.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/TimedCallback.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace nodediscovery {

#define DEFINE_ENUM NodePingResult
#define ENUM_LIST NODE_PING_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

	namespace {
		class NodePingRequest {
		public:
			NodePingRequest(
					const ionet::Node& requestNode,
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
					model::NetworkIdentifier networkIdentifier,
					const NodePingRequestor::PingCallback& pingCallback)
					: m_requestNode(requestNode)
					, m_pPool(pPool)
					, m_networkIdentifier(networkIdentifier)
					, m_pingCallback(pingCallback)
			{}

		public:
			void setTimeout(const utils::TimeSpan& timeout, const std::shared_ptr<ionet::PacketSocket>& pSocket) {
				auto pTimedCallback = thread::MakeTimedCallback(
						m_pPool->service(),
						m_pingCallback,
						NodePingResult::Failure_Timeout,
						ionet::Node());
				pTimedCallback->setTimeout(timeout);
				pTimedCallback->setTimeoutHandler([pSocket, requestNode = m_requestNode]() {
					pSocket->close();
					CATAPULT_LOG(debug) << "ping request connection to '" << requestNode << "' timed out";
				});
				m_pingCallback = [pTimedCallback](auto result, const auto& responseNode) {
					pTimedCallback->callback(result, responseNode);
				};
			}

		public:
			void complete(net::PeerConnectResult connectResult) {
				CATAPULT_LOG(debug) << "ping request connection to '" << m_requestNode << "' failed: " << connectResult;
				complete(NodePingResult::Failure_Connection);
			}

			void complete(thread::future<ionet::Node>&& nodeFuture) {
				try {
					const auto& responseNode = nodeFuture.get();
					if (!IsNodeCompatible(responseNode, m_networkIdentifier, m_requestNode.identityKey())) {
						CATAPULT_LOG(warning) << "rejecting incompatible partner node '" << responseNode << "'";
						return complete(NodePingResult::Failure_Incompatible);
					}

					complete(responseNode);
				} catch (const catapult_runtime_error& e) {
					CATAPULT_LOG(warning)
							<< "exception thrown while requesting node information from '"
							<< m_requestNode << "': " << e.what();
					complete(NodePingResult::Failure_Interaction);
				}
			}

		private:
			void complete(NodePingResult result) {
				m_pingCallback(result, ionet::Node());
			}

			void complete(const ionet::Node& responseNode) {
				m_pingCallback(NodePingResult::Success, responseNode);
			}

		private:
			ionet::Node m_requestNode;
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			model::NetworkIdentifier m_networkIdentifier;
			NodePingRequestor::PingCallback m_pingCallback;
		};

		class DefaultNodePingRequestor
				: public NodePingRequestor
				, public std::enable_shared_from_this<DefaultNodePingRequestor> {
		public:
			DefaultNodePingRequestor(
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
					const crypto::KeyPair& keyPair,
					const net::ConnectionSettings& settings,
					model::NetworkIdentifier networkIdentifier)
					: m_pPool(pPool)
					, m_networkIdentifier(networkIdentifier)
					, m_pingTimeout(settings.Timeout)
					, m_pConnector(net::CreateServerConnector(pPool, keyPair, settings))
					, m_numTotalPingRequests(0)
					, m_numSuccessfulPingRequests(0)
			{}

		public:
			size_t numActiveConnections() const override {
				return m_pConnector->numActiveConnections();
			}

			size_t numTotalPingRequests() const override {
				return m_numTotalPingRequests;
			}

			size_t numSuccessfulPingRequests() const override {
				return m_numSuccessfulPingRequests;
			}

		public:
			void requestPing(const ionet::Node& node, const PingCallback& callback) override {
				++m_numTotalPingRequests;
				auto wrappedCallback = [pThis = shared_from_this(), callback](auto result, const auto& responseNode) {
					if (NodePingResult::Success == result)
						++pThis->m_numSuccessfulPingRequests;

					callback(result, responseNode);
				};

				auto pRequest = std::make_shared<NodePingRequest>(node, m_pPool, m_networkIdentifier, wrappedCallback);
				m_pConnector->connect(node, [pRequest, pingTimeout = m_pingTimeout](auto connectResult, const auto& pSocket) {
					pRequest->setTimeout(pingTimeout, pSocket);

					if (net::PeerConnectResult::Accepted != connectResult)
						return pRequest->complete(connectResult);

					auto pApi = api::CreateRemoteNodeApi(*pSocket);
					pApi->nodeInfo().then([pSocket, pRequest](auto&& nodeFuture) {
						pRequest->complete(std::move(nodeFuture));
					});
				});
			}

			void shutdown() override {
				m_pConnector->shutdown();
			}

		private:
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			model::NetworkIdentifier m_networkIdentifier;
			utils::TimeSpan m_pingTimeout;
			std::shared_ptr<net::ServerConnector> m_pConnector;

			std::atomic<size_t> m_numTotalPingRequests;
			std::atomic<size_t> m_numSuccessfulPingRequests;
		};
	}

	std::shared_ptr<NodePingRequestor> CreateNodePingRequestor(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
			const crypto::KeyPair& keyPair,
			const net::ConnectionSettings& settings,
			model::NetworkIdentifier networkIdentifier) {
		return std::make_shared<DefaultNodePingRequestor>(pPool, keyPair, settings, networkIdentifier);
	}
}}
