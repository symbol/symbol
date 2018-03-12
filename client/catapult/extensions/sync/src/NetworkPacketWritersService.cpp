#include "NetworkPacketWritersService.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/extensions/ServiceUtils.h"
#include "catapult/ionet/BroadcastUtils.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace sync {

	namespace {
		constexpr auto Service_Name = "writers";
		using BlockSink = extensions::NewBlockSink;
		using TransactionsSink = extensions::SharedNewTransactionsSink;

		extensions::RemoteChainHeightsRetriever CreateRemoteChainHeightsRetriever(net::PacketIoPicker& packetIoPicker) {
			return [&packetIoPicker](auto numPeers) {
				std::vector<thread::future<Height>> heightFutures;
				auto timeout = utils::TimeSpan::FromSeconds(5);

				auto packetIoPairs = net::PickMultiple(packetIoPicker, numPeers, timeout);
				if (packetIoPairs.empty()) {
					CATAPULT_LOG(warning) << "could not find any peer for detecting chain heights";
					return thread::make_ready_future(std::vector<Height>());
				}

				for (const auto& packetIoPair : packetIoPairs) {
					auto pPacketIo = packetIoPair.io();
					auto pChainApi = api::CreateRemoteChainApiWithoutRegistry(*pPacketIo);
					heightFutures.push_back(pChainApi->chainInfo().then([pPacketIo](auto&& infoFuture) {
						return infoFuture.get().Height;
					}));
				}

				return thread::when_all(std::move(heightFutures)).then([](auto&& completedFutures) {
					return thread::get_all_ignore_exceptional(completedFutures.get());
				});
			};
		}

		class NetworkPacketWritersServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "NetworkPacketWriters", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<net::PacketWriters>(Service_Name, "WRITERS", [](const auto& writers) {
					return writers.numActiveWriters();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto connectionSettings = extensions::GetConnectionSettings(state.config());
				auto pServiceGroup = state.pool().pushServiceGroup(Service_Name);
				auto pWriters = pServiceGroup->pushService(net::CreatePacketWriters, locator.keyPair(), connectionSettings);

				locator.registerService(Service_Name, pWriters);
				state.packetIoPickers().insert(*pWriters, ionet::NodeRoles::Peer);

				// add sinks
				state.hooks().addNewBlockSink(extensions::CreatePushEntitySink<BlockSink>(locator, Service_Name));
				state.hooks().addNewTransactionsSink(extensions::CreatePushEntitySink<TransactionsSink>(locator, Service_Name));
				state.hooks().addPacketPayloadSink([&writers = *pWriters](const auto& payload) { writers.broadcast(payload); });

				// add retrievers
				state.hooks().setRemoteChainHeightsRetriever(CreateRemoteChainHeightsRetriever(*pWriters));
			}
		};
	}

	std::shared_ptr<net::PacketWriters> GetPacketWriters(const extensions::ServiceLocator& locator) {
		return locator.service<net::PacketWriters>(Service_Name);
	}

	DECLARE_SERVICE_REGISTRAR(NetworkPacketWriters)() {
		return std::make_unique<NetworkPacketWritersServiceRegistrar>();
	}
}}
