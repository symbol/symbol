#include "catapult/local/NetworkUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

	namespace {
		auto CreateLocalNodeConfiguration() {
			// Arrange:
			auto blockChainConfig = model::BlockChainConfiguration::Uninitialized();
			blockChainConfig.Network.Identifier = static_cast<model::NetworkIdentifier>(7);

			auto nodeConfig = config::NodeConfiguration::Uninitialized();
			nodeConfig.ShouldAllowAddressReuse = true;
			nodeConfig.ConnectTimeout = utils::TimeSpan::FromSeconds(11);
			nodeConfig.SocketWorkingBufferSize = utils::FileSize::FromBytes(512);
			nodeConfig.MaxPacketDataSize = utils::FileSize::FromKilobytes(12);

			return config::LocalNodeConfiguration(
					std::move(blockChainConfig),
					std::move(nodeConfig),
					config::LoggingConfiguration::Uninitialized(),
					config::UserConfiguration::Uninitialized(),
					{});
		}
	}

	// region GetConnectionSettings

	TEST(NetworkUtilsTests, CanExtractConnectionSettingsFromLocalNodeConfiguration) {
		// Arrange:
		auto config = CreateLocalNodeConfiguration();

		// Act:
		auto settings = GetConnectionSettings(config);

		// Assert:
		EXPECT_EQ(static_cast<model::NetworkIdentifier>(7), settings.NetworkIdentifier);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(11), settings.Timeout);
		EXPECT_EQ(utils::FileSize::FromBytes(512), settings.SocketWorkingBufferSize);
		EXPECT_EQ(utils::FileSize::FromKilobytes(12), settings.MaxPacketDataSize);
	}

	// endregion

	// region BootServer

	namespace {
		class MockAcceptor {
		public:
			MockAcceptor() : m_numAccepts(0)
			{}

		public:
			size_t numAccepts() const {
				return m_numAccepts;
			}

		public:
			void accept(
					const std::shared_ptr<net::AsyncTcpServerAcceptContext>&,
					const std::function<void (net::PeerConnectResult)>&) {
				++m_numAccepts;
			}

		private:
			std::atomic<size_t> m_numAccepts;
		};

		struct BootServerContext {
		public:
			BootServerContext() : m_pool(1, "network utils")
			{}

		public:
			size_t numAccepts() const {
				return m_acceptor.numAccepts();
			}

		public:
			auto boot() {
				// Act:
				auto config = CreateLocalNodeConfiguration();
				return BootServer(*m_pool.pushServiceGroup("server"), test::Local_Host_Port, config, m_acceptor);
			}

		private:
			MockAcceptor m_acceptor;
			thread::MultiServicePool m_pool;
		};
	}

	TEST(NetworkUtilsTests, CanBootServer) {
		// Arrange:
		BootServerContext context;

		// Act:
		auto pServer = context.boot();

		// Assert:
		EXPECT_EQ(0u, pServer->numLifetimeConnections());
		EXPECT_EQ(0u, context.numAccepts());
	}

	TEST(NetworkUtilsTests, CanConnectToBootedServer) {
		// Arrange: boot the server
		BootServerContext context;
		auto pServer = context.boot();

		// Act: connect to the server
		auto pClientThreadPool = test::CreateStartedIoServiceThreadPool(1);
		test::CreateClientSocket(pClientThreadPool->service())->connect().get();
		WAIT_FOR_ONE_EXPR(context.numAccepts());

		// Assert:
		EXPECT_EQ(1u, pServer->numLifetimeConnections());
		EXPECT_EQ(1u, context.numAccepts());
	}

	// endregion
}}
