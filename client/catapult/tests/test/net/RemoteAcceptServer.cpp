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

#include "RemoteAcceptServer.h"
#include "CertificateLocator.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/crypto/CertificateTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"

namespace catapult { namespace test {

	RemoteAcceptServer::RemoteAcceptServer() : RemoteAcceptServer(GenerateKeyPair())
	{}

	RemoteAcceptServer::RemoteAcceptServer(const crypto::KeyPair& caKeyPair)
			: m_directoryGuard("cert_ras")
			, m_pPool(CreateStartedIoThreadPool(1))
			, m_caKeyPair(CopyKeyPair(caKeyPair))
			, m_nodeKeyPair(GenerateKeyPair()) {
		GenerateCertificateDirectory(m_directoryGuard.name(), PemCertificate(m_caKeyPair, m_nodeKeyPair));
	}

	boost::asio::io_context& RemoteAcceptServer::ioContext() {
		return m_pPool->ioContext();
	}

	const Key& RemoteAcceptServer::caPublicKey() const {
		return m_caKeyPair.publicKey();
	}

	void RemoteAcceptServer::start() {
		start([](const auto&) {});
	}

	void RemoteAcceptServer::start(const PacketSocketWork& serverWork) {
		auto options = CreatePacketSocketOptions();
		options.SslOptions.ContextSupplier = ionet::CreateSslContextSupplier(m_directoryGuard.name());
		SpawnPacketServerWork(m_pPool->ioContext(), options, serverWork);
	}

	void RemoteAcceptServer::start(const TcpAcceptor& acceptor, const PacketSocketWork& serverWork) {
		auto options = CreatePacketSocketOptions();
		options.SslOptions.ContextSupplier = ionet::CreateSslContextSupplier(m_directoryGuard.name());
		SpawnPacketServerWork(acceptor, options, serverWork);
	}

	void RemoteAcceptServer::join() {
		m_pPool->join();
	}
}}
