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
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/other/MutableCatapultConfiguration.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS NetworkUtilsTests

	namespace {
		auto CreateCatapultConfiguration() {
			test::MutableCatapultConfiguration config;
			config.BlockChain.Network.Identifier = static_cast<model::NetworkIdentifier>(7);

			config.Node.ConnectTimeout = utils::TimeSpan::FromSeconds(11);
			config.Node.SocketWorkingBufferSize = utils::FileSize::FromBytes(512);
			config.Node.SocketWorkingBufferSensitivity = 987;
			config.Node.MaxPacketDataSize = utils::FileSize::FromKilobytes(12);

			config.Node.IncomingConnections.MaxConnections = 17;
			config.Node.IncomingConnections.BacklogSize = 83;
			config.Node.EnableAddressReuse = true;
			config.Node.OutgoingSecurityMode = static_cast<ionet::ConnectionSecurityMode>(8);
			config.Node.IncomingSecurityModes = static_cast<ionet::ConnectionSecurityMode>(21);
			return config.ToConst();
		}
	}

	// region GetConnectionSettings / UpdateAsyncTcpServerSettings

	TEST(TEST_CLASS, CanExtractConnectionSettingsFromCatapultConfiguration) {
		// Arrange:
		auto config = CreateCatapultConfiguration();

		// Act:
		auto settings = GetConnectionSettings(config);

		// Assert:
		EXPECT_EQ(static_cast<model::NetworkIdentifier>(7), settings.NetworkIdentifier);
		EXPECT_EQ(utils::TimeSpan::FromSeconds(11), settings.Timeout);
		EXPECT_EQ(utils::FileSize::FromBytes(512), settings.SocketWorkingBufferSize);
		EXPECT_EQ(987u, settings.SocketWorkingBufferSensitivity);
		EXPECT_EQ(utils::FileSize::FromKilobytes(12), settings.MaxPacketDataSize);

		EXPECT_EQ(static_cast<ionet::ConnectionSecurityMode>(8), settings.OutgoingSecurityMode);
		EXPECT_EQ(static_cast<ionet::ConnectionSecurityMode>(21), settings.IncomingSecurityModes);
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

	// region GetMaxIncomingConnectionsPerIdentity

	TEST(TEST_CLASS, GetMaxIncomingConnectionsPerIdentityReturnsCorrectValueBasedOnRoles) {
		EXPECT_EQ(1u, GetMaxIncomingConnectionsPerIdentity(ionet::NodeRoles::None));
		EXPECT_EQ(2u, GetMaxIncomingConnectionsPerIdentity(ionet::NodeRoles::Peer));
		EXPECT_EQ(2u, GetMaxIncomingConnectionsPerIdentity(ionet::NodeRoles::Api));
		EXPECT_EQ(3u, GetMaxIncomingConnectionsPerIdentity(ionet::NodeRoles::Peer | ionet::NodeRoles::Api));
		EXPECT_EQ(3u, GetMaxIncomingConnectionsPerIdentity(static_cast<ionet::NodeRoles>(0xFFFFFFFF)));
	}

	// endregion

	// region BootServer

	namespace {
		class MockAcceptor {
		public:
			MockAcceptor(net::PeerConnectCode connectCode, const Key& identityKey)
					: m_connectCode(connectCode)
					, m_identityKey(identityKey)
					, m_numAccepts(0)
			{}

		public:
			size_t numAccepts() const {
				return m_numAccepts;
			}

		public:
			void accept(const ionet::AcceptedPacketSocketInfo&, const consumer<net::PeerConnectResult>& consumer) {
				consumer({ m_connectCode, m_identityKey });
				++m_numAccepts;
			}

		private:
			net::PeerConnectCode m_connectCode;
			Key m_identityKey;
			std::atomic<size_t> m_numAccepts;
		};

		struct BootServerContext {
		public:
			BootServerContext(net::PeerConnectCode connectCode, const Key& identityKey)
					: m_acceptor(connectCode, identityKey)
					, m_pool("network utils", 1)
			{}

		public:
			size_t numAccepts() const {
				return m_acceptor.numAccepts();
			}

			const auto& nodeSubscriber() const {
				return m_nodeSubscriber;
			}

		public:
			auto boot() {
				// Act:
				auto& serviceGroup = *m_pool.pushServiceGroup("server");
				auto config = CreateCatapultConfiguration();
				auto serviceId = ionet::ServiceIdentifier(123);
				auto& acceptor = m_acceptor;
				return BootServer(serviceGroup, test::GetLocalHostPort(), serviceId, config, m_nodeSubscriber, [&acceptor](
						const auto& socketInfo,
						const auto& callback) {
					acceptor.accept(socketInfo, callback);
				});
			}

		private:
			MockAcceptor m_acceptor;
			thread::MultiServicePool m_pool;
			mocks::MockNodeSubscriber m_nodeSubscriber;
		};
	}

	TEST(TEST_CLASS, CanBootServer) {
		// Arrange:
		auto key = test::GenerateRandomByteArray<Key>();
		BootServerContext context(net::PeerConnectCode::Accepted, key);

		// Act:
		auto pServer = context.boot();

		// Assert:
		EXPECT_EQ(0u, pServer->numLifetimeConnections());
		EXPECT_EQ(0u, context.numAccepts());

		const auto& params = context.nodeSubscriber().incomingNodeParams().params();
		EXPECT_EQ(0u, params.size());
	}

	TEST(TEST_CLASS, CanConnectToBootedServer_AcceptFails) {
		// Arrange: boot the server
		auto key = test::GenerateRandomByteArray<Key>();
		BootServerContext context(net::PeerConnectCode::Socket_Error, key);
		auto pServer = context.boot();

		// Act: connect to the server
		auto pClientThreadPool = test::CreateStartedIoThreadPool(1);
		test::CreateClientSocket(pClientThreadPool->ioContext())->connect().get();
		WAIT_FOR_ONE_EXPR(context.numAccepts());

		// Assert:
		EXPECT_EQ(1u, pServer->numLifetimeConnections());
		EXPECT_EQ(1u, context.numAccepts());

		const auto& params = context.nodeSubscriber().incomingNodeParams().params();
		EXPECT_EQ(0u, params.size());
	}

	TEST(TEST_CLASS, CanConnectToBootedServer_AcceptSucceeds) {
		// Arrange: boot the server
		auto key = test::GenerateRandomByteArray<Key>();
		BootServerContext context(net::PeerConnectCode::Accepted, key);
		auto pServer = context.boot();

		// Act: connect to the server
		auto pClientThreadPool = test::CreateStartedIoThreadPool(1);
		test::CreateClientSocket(pClientThreadPool->ioContext())->connect().get();
		WAIT_FOR_ONE_EXPR(context.numAccepts());

		// Assert:
		EXPECT_EQ(1u, pServer->numLifetimeConnections());
		EXPECT_EQ(1u, context.numAccepts());

		const auto& params = context.nodeSubscriber().incomingNodeParams().params();
		ASSERT_EQ(1u, params.size());
		EXPECT_EQ(key, params[0].IdentityKey);
		EXPECT_EQ(ionet::ServiceIdentifier(123), params[0].ServiceId); // from BootServerContext::boot
	}

	// endregion
}}
