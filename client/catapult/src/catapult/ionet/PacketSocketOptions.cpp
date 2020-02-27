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

#include "PacketSocketOptions.h"
#include "catapult/crypto/CatapultCertificateProcessor.h"
#include <boost/asio/ssl.hpp>

namespace catapult { namespace ionet {

	PacketSocketSslVerifyContext::PacketSocketSslVerifyContext()
			: m_preverified(false)
			, m_pVerifyContext(nullptr)
			, m_pPublicKey(&m_publicKeyBacking)
	{}

	PacketSocketSslVerifyContext::PacketSocketSslVerifyContext(
			bool preverified,
			boost::asio::ssl::verify_context& verifyContext,
			Key& publicKey)
			: m_preverified(preverified)
			, m_pVerifyContext(&verifyContext)
			, m_pPublicKey(&publicKey)
	{}

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

	supplier<boost::asio::ssl::context&> CreateSslContextSupplier(const boost::filesystem::path& certificateDirectory) {
		auto pSslContext = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv13);
		pSslContext->set_options(
				boost::asio::ssl::context::single_dh_use
				| boost::asio::ssl::context::no_sslv2
				| boost::asio::ssl::context::no_sslv3
				| boost::asio::ssl::context::no_tlsv1
				| boost::asio::ssl::context::no_tlsv1_1
				| boost::asio::ssl::context::no_tlsv1_2);

		pSslContext->use_certificate_chain_file((certificateDirectory / "node.full.crt.pem").generic_string());
		pSslContext->use_private_key_file((certificateDirectory / "node.key.pem").generic_string(), boost::asio::ssl::context::pem);
		pSslContext->use_tmp_dh_file((certificateDirectory / "dhparam.pem").generic_string());
		return [pSslContext]() -> boost::asio::ssl::context& {
			return *pSslContext;
		};
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
