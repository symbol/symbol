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

#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/Results.h"
#include "catapult/net/ConnectionContainer.h"
#include "catapult/net/PeerConnectResult.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/crypto/CertificateTestUtils.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/nodeps/TimeSupplier.h"
#include "tests/test/other/MutableCatapultConfiguration.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/TestHarness.h"
#include <boost/asio/ssl.hpp>

namespace catapult { namespace extensions {

#define TEST_CLASS NetworkUtilsTests

	// region test utils (CreateCatapultConfiguration)

	namespace {
		auto CreateCatapultConfiguration(bool enableReadRateMonitoring = false, const std::string& listenInterface = std::string()) {
			test::MutableCatapultConfiguration config;
			config.User.CertificateDirectory = test::GetDefaultCertificateDirectory();

			config.BlockChain.Network.Identifier = static_cast<model::NetworkIdentifier>(7);
			config.BlockChain.Network.NodeEqualityStrategy = static_cast<model::NodeIdentityEqualityStrategy>(11);

			config.Node.EnableAddressReuse = true;
			config.Node.ConnectTimeout = utils::TimeSpan::FromSeconds(11);
			config.Node.SocketWorkingBufferSize = utils::FileSize::FromBytes(512);
			config.Node.SocketWorkingBufferSensitivity = 987;
			config.Node.MaxPacketDataSize = utils::FileSize::FromKilobytes(12);
			config.Node.ListenInterface = listenInterface;

			config.Node.IncomingConnections.MaxConnections = 17;
			config.Node.IncomingConnections.BacklogSize = 83;

			config.Node.Banning.DefaultBanDuration = utils::TimeSpan::FromHours(1);
			config.Node.Banning.MaxBannedNodes = 50;

			config.Node.Banning.NumReadRateMonitoringBuckets = enableReadRateMonitoring ? 4 : 0;
			config.Node.Banning.ReadRateMonitoringBucketDuration = utils::TimeSpan::FromSeconds(15);
			config.Node.Banning.MaxReadRateMonitoringTotalSize = utils::FileSize::FromBytes(1024);

			return config.ToConst();
		}
	}

	// endregion

	// region GetRateMonitorSettings

	TEST(TEST_CLASS, CanGetRateMonitorSettingsFromBanningSubConfiguration) {
		// Arrange:
		config::NodeConfiguration::BanningSubConfiguration banConfig;
		banConfig.NumReadRateMonitoringBuckets = 12;
		banConfig.ReadRateMonitoringBucketDuration = utils::TimeSpan::FromMinutes(23);
		banConfig.MaxReadRateMonitoringTotalSize = utils::FileSize::FromKilobytes(34);

		// Act:
		auto rateMonitorSettings = GetRateMonitorSettings(banConfig);

		// Assert:
		EXPECT_EQ(12u, rateMonitorSettings.NumBuckets);
		EXPECT_EQ(utils::TimeSpan::FromMinutes(23), rateMonitorSettings.BucketDuration);
		EXPECT_EQ(utils::FileSize::FromKilobytes(34), rateMonitorSettings.MaxTotalSize);
	}

	// endregion

	// region GetConnectionSettings / UpdateAsyncTcpServerSettings

	namespace {
		bool RunVerifyCallback(const predicate<ionet::PacketSocketSslVerifyContext&>& verifyCallback) {
			// Arrange:
			test::CertificateBuilder builder;
			builder.setSubject("JP", "NEM", "Node");
			builder.setIssuer("JP", "NEM", "Root");
			builder.setPublicKey(*test::GenerateRandomCertificateKey());

			std::vector<test::CertificatePointer> certificates;
			certificates.push_back(builder.build());
			auto holder = test::CreateCertificateStoreContextFromCertificates(std::move(certificates));

			boost::asio::ssl::verify_context asioVerifyContext(holder.pCertificateStoreContext.get());
			Key publicKey;
			ionet::PacketSocketSslVerifyContext verifyContext(false, asioVerifyContext, publicKey);

			// Act:
			return verifyCallback(verifyContext);
		}
	}

	TEST(TEST_CLASS, CanExtractConnectionSettingsFromCatapultConfiguration) {
		// Arrange:
		auto config = CreateCatapultConfiguration();

		// Act:
		auto settings = GetConnectionSettings(config);

		// Assert:
		EXPECT_EQ(static_cast<model::NetworkIdentifier>(7), settings.NetworkIdentifier);
		EXPECT_EQ(static_cast<model::NodeIdentityEqualityStrategy>(11), settings.NodeIdentityEqualityStrategy);

		EXPECT_EQ(utils::TimeSpan::FromSeconds(11), settings.Timeout);
		EXPECT_EQ(utils::FileSize::FromBytes(512), settings.SocketWorkingBufferSize);
		EXPECT_EQ(987u, settings.SocketWorkingBufferSensitivity);
		EXPECT_EQ(utils::FileSize::FromKilobytes(12), settings.MaxPacketDataSize);

		EXPECT_TRUE(settings.AllowIncomingSelfConnections);
		EXPECT_FALSE(settings.AllowOutgoingSelfConnections);

		EXPECT_NO_THROW(settings.SslOptions.ContextSupplier());
		EXPECT_FALSE(RunVerifyCallback(settings.SslOptions.VerifyCallbackSupplier()));
	}

	TEST(TEST_CLASS, CanUpdateAsyncTcpServerSettingsFromCatapultConfiguration) {
		// Arrange:
		auto config = CreateCatapultConfiguration();
		auto settings = net::AsyncTcpServerSettings([](const auto&) {});

		// Act:
		UpdateAsyncTcpServerSettings(settings, config);

		// Assert:
		EXPECT_EQ(512u, settings.PacketSocketOptions.WorkingBufferSize);
		EXPECT_EQ(987u, settings.PacketSocketOptions.WorkingBufferSensitivity);
		EXPECT_EQ(12u * 1024, settings.PacketSocketOptions.MaxPacketDataSize);

		EXPECT_EQ(17u, settings.MaxActiveConnections);
		EXPECT_EQ(83u, settings.MaxPendingConnections);
		EXPECT_TRUE(settings.AllowAddressReuse);
	}

	// endregion

	// region BootServer - test utils

	namespace {
		class MockAcceptor : public net::AcceptedConnectionContainer {
		public:
			MockAcceptor(net::PeerConnectCode connectCode, const model::NodeIdentity& identity)
					: m_connectCode(connectCode)
					, m_identity(identity)
					, m_numAccepts(0)
					, m_numCloses(0)
			{}

		public:
			size_t numAccepts() const {
				return m_numAccepts;
			}

			size_t numCloses() const {
				return m_numCloses;
			}

			const auto& callbackResults() const {
				return m_callbackResults;
			}

			const auto& socketInfos() const {
				return m_socketInfos;
			}

			const auto& closedIdentities() const {
				return m_closedIdentities;
			}

		public:
			void release() {
				m_socketInfos.clear();
			}

		public:
			size_t numActiveConnections() const override {
				CATAPULT_THROW_RUNTIME_ERROR("numActiveConnections - not supported in mock");
			}

			model::NodeIdentitySet identities() const override {
				CATAPULT_THROW_RUNTIME_ERROR("identities - not supported in mock");
			}

		public:
			void accept(const ionet::PacketSocketInfo& socketInfo, const AcceptCallback& callback) override {
				m_socketInfos.push_back(socketInfo);
				m_callbackResults.push_back(callback(net::PeerConnectResult(m_connectCode, m_identity)));
				++m_numAccepts;
			}

			bool closeOne(const model::NodeIdentity& identity) override {
				m_closedIdentities.push_back(identity);
				++m_numCloses;
				return true;
			}

		private:
			net::PeerConnectCode m_connectCode;
			model::NodeIdentity m_identity;
			std::atomic<size_t> m_numAccepts;
			std::atomic<size_t> m_numCloses;
			std::vector<bool> m_callbackResults;
			std::vector<ionet::PacketSocketInfo> m_socketInfos;
			std::vector<model::NodeIdentity> m_closedIdentities;
		};

		struct BootServerContext {
		public:
			BootServerContext(net::PeerConnectCode connectCode, const Key& identityKey)
					: m_acceptor(connectCode, { identityKey, "11.22.33.44" })
					, m_pool("network utils", 1)
			{}

			~BootServerContext() {
				// captured PacketSocketInfos need to be released before m_pool can be destroyed
				m_acceptor.release();
			}

		public:
			const auto& acceptor() const {
				return m_acceptor;
			}

			const auto& nodeSubscriber() const {
				return m_nodeSubscriber;
			}

		public:
			void setNodeSubscriberFailure() {
				m_nodeSubscriber.setNotifyIncomingNodeResult(false);
			}

		public:
			auto boot(const config::CatapultConfiguration& config) {
				// Act:
				auto& serviceGroup = *m_pool.pushServiceGroup("server");
				auto serviceId = ionet::ServiceIdentifier(123);
				auto timeSupplier = test::CreateTimeSupplierFromMilliseconds({ 1 });
				return BootServer(serviceGroup, test::GetLocalHostPort(), serviceId, config, timeSupplier, m_nodeSubscriber, m_acceptor);
			}

			auto boot() {
				return boot(CreateCatapultConfiguration());
			}

		private:
			MockAcceptor m_acceptor;
			thread::MultiServicePool m_pool;
			mocks::MockNodeSubscriber m_nodeSubscriber;
		};

		enum class ConnectionFlags { None = 0, Is_Closed = 1, Is_Banned = 2, Is_Callback_Failure = 4 };

		constexpr ConnectionFlags operator|(ConnectionFlags lhs, ConnectionFlags rhs) {
			return static_cast<ConnectionFlags>(utils::to_underlying_type(lhs) | utils::to_underlying_type(rhs));
		}

		constexpr bool HasFlag(ConnectionFlags testedFlag, ConnectionFlags value) {
			return utils::to_underlying_type(testedFlag) == (utils::to_underlying_type(testedFlag) & utils::to_underlying_type(value));
		}

		void AssertAcceptedConnection(
				const BootServerContext& context,
				const Key& key,
				const net::AsyncTcpServer& server,
				ConnectionFlags flags = ConnectionFlags::None) {
			EXPECT_EQ(1u, server.numLifetimeConnections());
			EXPECT_EQ(1u, context.acceptor().numAccepts());
			EXPECT_EQ(HasFlag(ConnectionFlags::Is_Closed, flags) ? 1u : 0u, context.acceptor().closedIdentities().size());

			const auto& callbackResults = context.acceptor().callbackResults();
			ASSERT_EQ(1u, callbackResults.size());
			EXPECT_EQ(!HasFlag(ConnectionFlags::Is_Callback_Failure, flags), callbackResults[0]);

			const auto& params = context.nodeSubscriber().incomingNodeParams().params();
			ASSERT_EQ(1u, params.size());
			EXPECT_EQ(key, params[0].Identity.PublicKey);
			EXPECT_EQ("11.22.33.44", params[0].Identity.Host);
			EXPECT_EQ(ionet::ServiceIdentifier(123), params[0].ServiceId); // from BootServerContext::boot

			const auto& banParams = context.nodeSubscriber().banParams().params();
			EXPECT_EQ(HasFlag(ConnectionFlags::Is_Banned, flags) ? 1u : 0u, banParams.size());
		}
	}

	// endregion

	// region BootServer - basic

	TEST(TEST_CLASS, BootServer_CanCreateServer) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		BootServerContext context(net::PeerConnectCode::Accepted, key);

		// Act:
		auto pServer = context.boot();

		// Assert:
		EXPECT_EQ(0u, pServer->numLifetimeConnections());
		EXPECT_EQ(0u, context.acceptor().numAccepts());
		EXPECT_EQ(0u, context.acceptor().closedIdentities().size());

		const auto& callbackResults = context.acceptor().callbackResults();
		EXPECT_EQ(0u, callbackResults.size());

		const auto& params = context.nodeSubscriber().incomingNodeParams().params();
		EXPECT_EQ(0u, params.size());

		const auto& banParams = context.nodeSubscriber().banParams().params();
		EXPECT_EQ(0u, banParams.size());
	}

	TEST(TEST_CLASS, BootServer_ConnectionRejectedWhenAcceptFails) {
		// Arrange: boot the server
		auto key = test::GenerateRandomByteArray<Key>();
		BootServerContext context(net::PeerConnectCode::Socket_Error, key);
		auto pServer = context.boot();

		// Act: connect to the server
		auto pClientThreadPool = test::CreateStartedIoThreadPool(1);
		auto pClientSocket = test::AddClientConnectionTask(pClientThreadPool->ioContext());
		WAIT_FOR_ONE_EXPR(context.acceptor().numAccepts());

		// Assert:
		EXPECT_EQ(1u, pServer->numLifetimeConnections());
		EXPECT_EQ(1u, context.acceptor().numAccepts());
		EXPECT_EQ(0u, context.acceptor().closedIdentities().size());

		const auto& callbackResults = context.acceptor().callbackResults();
		ASSERT_EQ(1u, callbackResults.size());
		EXPECT_FALSE(callbackResults[0]);

		const auto& params = context.nodeSubscriber().incomingNodeParams().params();
		EXPECT_EQ(0u, params.size());

		const auto& banParams = context.nodeSubscriber().banParams().params();
		EXPECT_EQ(0u, banParams.size());
	}

	namespace {
		void AssertBootServerConnectionAcceptedWhenAcceptSucceedsWithNotifySuccess(const std::string& listenInterface) {
			// Arrange: boot the server
			auto key = test::GenerateRandomByteArray<Key>();
			BootServerContext context(net::PeerConnectCode::Accepted, key);
			auto pServer = context.boot(CreateCatapultConfiguration(false, listenInterface));

			// Act: connect to the server
			auto pClientThreadPool = test::CreateStartedIoThreadPool(1);
			auto pClientSocket = test::AddClientConnectionTask(pClientThreadPool->ioContext());
			WAIT_FOR_ONE_EXPR(context.acceptor().numAccepts());

			// Assert:
			AssertAcceptedConnection(context, key, *pServer);
		}

		void AssertBootServerConnectionRejectedWhenAcceptSucceedsWithNotifyFailure(const std::string& listenInterface) {
			// Arrange: boot the server
			auto key = test::GenerateRandomByteArray<Key>();
			BootServerContext context(net::PeerConnectCode::Accepted, key);
			context.setNodeSubscriberFailure();
			auto pServer = context.boot(CreateCatapultConfiguration(false, listenInterface));

			// Act: connect to the server
			auto pClientThreadPool = test::CreateStartedIoThreadPool(1);
			auto pClientSocket = test::AddClientConnectionTask(pClientThreadPool->ioContext());
			WAIT_FOR_ONE_EXPR(context.acceptor().numAccepts());

			// Assert:
			AssertAcceptedConnection(context, key, *pServer, ConnectionFlags::Is_Closed | ConnectionFlags::Is_Callback_Failure);
		}
	}

	TEST(TEST_CLASS, BootServer_ConnectionAcceptedWhenAcceptSucceedsWithNotifySuccess) {
		AssertBootServerConnectionAcceptedWhenAcceptSucceedsWithNotifySuccess("");
	}

	TEST(TEST_CLASS, BootServer_ConnectionRejectedWhenAcceptSucceedsWithNotifyFailure) {
		AssertBootServerConnectionRejectedWhenAcceptSucceedsWithNotifyFailure("");
	}

	TEST(TEST_CLASS, BootServer_ConnectionAcceptedWhenAcceptSucceedsWithNotifySuccess_WithCustomListenInterface) {
		AssertBootServerConnectionAcceptedWhenAcceptSucceedsWithNotifySuccess("127.0.0.1");
	}

	TEST(TEST_CLASS, BootServer_ConnectionRejectedWhenAcceptSucceedsWithNotifyFailure_WithCustomListenInterface) {
		AssertBootServerConnectionRejectedWhenAcceptSucceedsWithNotifyFailure("127.0.0.1");
	}

	// endregion

	// region BootServer - rate limiting

	namespace {
		template<typename TAction>
		void RunRateLimitingTest(bool enableReadRateMonitoring, uint32_t bufferSize, TAction action) {
			// Arrange: boot the server
			auto key = test::GenerateRandomByteArray<Key>();
			BootServerContext context(net::PeerConnectCode::Accepted, key);
			auto pServer = context.boot(CreateCatapultConfiguration(enableReadRateMonitoring));

			// Act: connect to the server and send a packet
			auto writeBuffer = test::GenerateRandomPacketBuffer(bufferSize);
			auto pClientThreadPool = test::CreateStartedIoThreadPool(1);
			auto pClientSocket = test::AddClientWriteBuffersTask(pClientThreadPool->ioContext(), { writeBuffer });
			WAIT_FOR_ONE_EXPR(context.acceptor().numAccepts());

			ASSERT_EQ(1u, context.acceptor().socketInfos().size());
			context.acceptor().socketInfos()[0].socket()->read([](auto code, const auto* pPacket) {
				CATAPULT_LOG(debug) << "read completed with code " << code << " (size = " << (pPacket ? pPacket->Size : 0) << ")";
			});

			// Assert:
			action(context, key, *pServer);
		}
	}

	TEST(TEST_CLASS, BootServer_AcceptedConnectionIsNotClosedWhenRateLimitIsExceededAndRateLimiterIsDisabled) {
		// Assert: connect to the server and send too much data (limit is 1024)
		RunRateLimitingTest(false, 1025, [](const auto& context, const auto& key, const auto& server) {
			AssertAcceptedConnection(context, key, server);
		});
	}

	TEST(TEST_CLASS, BootServer_AcceptedConnectionIsNotClosedWhenRateLimitIsNotExceededAndRateLimiterIsEnabled) {
		// Assert: connect to the server and send maximal data (limit is 1024)
		RunRateLimitingTest(true, 1024, [](const auto& context, const auto& key, const auto& server) {
			AssertAcceptedConnection(context, key, server);
		});
	}

	TEST(TEST_CLASS, BootServer_AcceptedConnectionIsClosedWhenRateLimitIsExceededAndRateLimiterIsEnabled) {
		// Assert: connect to the server and send too much data (limit is 1024)
		RunRateLimitingTest(true, 1025, [](const auto& context, const auto& key, const auto& server) {
			WAIT_FOR_ONE_EXPR(context.acceptor().numCloses());

			AssertAcceptedConnection(context, key, server, ConnectionFlags::Is_Closed | ConnectionFlags::Is_Banned);

			const auto& banParams = context.nodeSubscriber().banParams().params();
			ASSERT_EQ(1u, banParams.size());
			EXPECT_EQ(key, banParams[0].Identity.PublicKey);
			EXPECT_EQ("11.22.33.44", banParams[0].Identity.Host);
			EXPECT_EQ(Failure_Extension_Read_Rate_Limit_Exceeded, static_cast<validators::ValidationResult>(banParams[0].Reason));
		});
	}

	// endregion
}}
