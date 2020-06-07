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

#include "ExternalSourceConnection.h"
#include "LocalNodeTestUtils.h"
#include "plugins/coresystem/src/plugins/VrfKeyLinkTransactionPlugin.h"
#include "plugins/txes/mosaic/src/plugins/MosaicDefinitionTransactionPlugin.h"
#include "plugins/txes/mosaic/src/plugins/MosaicSupplyChangeTransactionPlugin.h"
#include "plugins/txes/namespace/src/plugins/MosaicAliasTransactionPlugin.h"
#include "plugins/txes/namespace/src/plugins/NamespaceRegistrationTransactionPlugin.h"
#include "plugins/txes/transfer/src/plugins/TransferTransactionPlugin.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/ServerConnector.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/crypto/CertificateTestUtils.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/other/RemoteApiFactory.h"

namespace catapult { namespace test {

	ExternalSourceConnection::ExternalSourceConnection(const Key& key)
			: ExternalSourceConnection(CreateLocalHostNode(key, GetLocalHostPort()))
	{}

	ExternalSourceConnection::ExternalSourceConnection(const ionet::Node& node)
			: m_pPool(CreateStartedIoThreadPool(1))
			, m_caKeyPair(crypto::KeyPair::FromPrivate(GenerateRandomPrivateKey()))
			, m_tempDirectoryGuard(ToString(m_caKeyPair.publicKey()))
			, m_pConnector(net::CreateServerConnector(*m_pPool, m_caKeyPair.publicKey(), createConnectionSettings(), "external source"))
			, m_localNode(node)
	{}

	std::shared_ptr<ionet::PacketIo> ExternalSourceConnection::io() const {
		return m_pIo;
	}

	void ExternalSourceConnection::connect(const consumer<const std::shared_ptr<ionet::PacketSocket>&>& onConnect) {
		m_pConnector->connect(m_localNode, [&pIo = m_pIo, onConnect](auto connectCode, const auto& socketInfo) {
			// save pIo in a member to tie the lifetime of the connection to the lifetime of the owning ExternalSourceConnection
			pIo = socketInfo.socket();
			if (net::PeerConnectCode::Accepted == connectCode)
				onConnect(socketInfo.socket());
		});
	}

	void ExternalSourceConnection::apiCall(const consumer<const std::shared_ptr<api::RemoteChainApi>&>& onConnect) {
		connect([onConnect](const auto& pPacketIo) {
			auto pRemoteApi = CreateLifetimeExtendedApi(
					api::CreateRemoteChainApi,
					pPacketIo,
					model::NodeIdentity(),
					CreateTransactionRegistry());
			onConnect(pRemoteApi);
		});
	}

	net::ConnectionSettings ExternalSourceConnection::createConnectionSettings() {
		GenerateCertificateDirectory(m_tempDirectoryGuard.name(), PemCertificate(m_caKeyPair, GenerateKeyPair()));

		auto settings = CreateConnectionSettings();
		settings.SslOptions.ContextSupplier = ionet::CreateSslContextSupplier(m_tempDirectoryGuard.name());
		settings.SslOptions.VerifyCallbackSupplier = ionet::CreateSslVerifyCallbackSupplier();
		return settings;
	}

	model::TransactionRegistry ExternalSourceConnection::CreateTransactionRegistry() {
		auto registry = model::TransactionRegistry();
		registry.registerPlugin(plugins::CreateMosaicDefinitionTransactionPlugin(plugins::MosaicRentalFeeConfiguration()));
		registry.registerPlugin(plugins::CreateMosaicSupplyChangeTransactionPlugin());
		registry.registerPlugin(plugins::CreateMosaicAliasTransactionPlugin());
		registry.registerPlugin(plugins::CreateNamespaceRegistrationTransactionPlugin(plugins::NamespaceRentalFeeConfiguration()));
		registry.registerPlugin(plugins::CreateTransferTransactionPlugin());
		registry.registerPlugin(plugins::CreateVrfKeyLinkTransactionPlugin());
		return registry;
	}
}}
