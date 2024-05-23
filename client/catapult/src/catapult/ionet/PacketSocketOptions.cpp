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

#include "PacketSocketOptions.h"
#include "catapult/crypto/CatapultCertificateProcessor.h"
#include "catapult/exceptions.h"
#include <boost/asio/ssl.hpp>

namespace catapult { namespace ionet {

	PacketSocketSslVerifyContext::PacketSocketSslVerifyContext()
			: m_preverified(false)
			, m_pVerifyContext(nullptr)
			, m_pPublicKey(&m_publicKeyBacking) {
	}

	PacketSocketSslVerifyContext::PacketSocketSslVerifyContext(
			bool preverified,
			boost::asio::ssl::verify_context& verifyContext,
			Key& publicKey)
			: m_preverified(preverified)
			, m_pVerifyContext(&verifyContext)
			, m_pPublicKey(&publicKey) {
	}

	bool PacketSocketSslVerifyContext::preverified() const {
		return m_preverified;
	}

	boost::asio::ssl::verify_context& PacketSocketSslVerifyContext::asioVerifyContext() {
		return *m_pVerifyContext;
	}

	const Key& PacketSocketSslVerifyContext::publicKey() const {
		return *m_pPublicKey;
	}

	void PacketSocketSslVerifyContext::setPublicKey(const Key& publicKey) {
		*m_pPublicKey = publicKey;
	}

	supplier<boost::asio::ssl::context&> CreateSslContextSupplier(const std::filesystem::path& certificateDirectory) {
		auto pSslContext = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13);
		pSslContext->set_options(
				boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::no_sslv3 | boost::asio::ssl::context::no_tlsv1
				| boost::asio::ssl::context::no_tlsv1_1 | boost::asio::ssl::context::no_tlsv1_2 | SSL_OP_CIPHER_SERVER_PREFERENCE);

		if (!SSL_CTX_set_num_tickets(pSslContext->native_handle(), 0))
			CATAPULT_THROW_RUNTIME_ERROR("failed to set the number of server tickets");

		pSslContext->use_certificate_chain_file((certificateDirectory / "node.full.crt.pem").generic_string());
		pSslContext->use_private_key_file((certificateDirectory / "node.key.pem").generic_string(), boost::asio::ssl::context::pem);

		std::array<int, 1> curves{ NID_X25519 };
		SSL_CTX_set1_groups(pSslContext->native_handle(), curves.data(), static_cast<long>(curves.size()));

		return [pSslContext]() -> boost::asio::ssl::context& { return *pSslContext; };
	}

	supplier<predicate<PacketSocketSslVerifyContext&>> CreateSslVerifyCallbackSupplier() {
		return []() {
			crypto::CatapultCertificateProcessor processor;
			return [processor](auto& verifyContext) mutable {
				if (!processor.verify(verifyContext.preverified(), *verifyContext.asioVerifyContext().native_handle()))
					return false;

				if (processor.size() > 0)
					verifyContext.setPublicKey(processor.certificate(0).PublicKey);

				return true;
			};
		};
	}
}}
