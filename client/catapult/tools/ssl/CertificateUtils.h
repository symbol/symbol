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

#pragma once
#include "catapult/crypto/KeyPair.h"
#include <memory>

struct evp_pkey_st;
struct x509_st;

namespace catapult { namespace tools { namespace ssl {

	// region key utils

	/// Creates pem private key.
	std::string CreatePrivateKeyPem(const crypto::KeyPair& keyPair);

	/// Creates a certificate chain from \a certificates.
	std::string CreateFullCertificateChainPem(const std::vector<std::shared_ptr<x509_st>>& certificates);

	// endregion

	// region CertificateBuilder

	/// Builder for x509 certificates.
	class CertificateBuilder {
	public:
		/// Creates a builder.
		CertificateBuilder();

	public:
		/// Adds the specified \a country, \a organization and \a commonName fields to the subject.
		void setSubject(const std::string& country, const std::string& organization, const std::string& commonName);

		/// Adds the specified \a country, \a organization and \a commonName fields to the issuer.
		void setIssuer(const std::string& country, const std::string& organization, const std::string& commonName);

		/// Sets the public key to \a publicKey.
		void setPublicKey(const Key& publicKey);

		/// Sets the validity of certificate to span from \a startDate to \a endDate.
		void setValidity(long startDate, long endDate);

	public:
		/// Builds the certificate.
		std::shared_ptr<x509_st> build();

		/// Builds and signs the certificate with \a keyPair.
		std::shared_ptr<x509_st> buildAndSign(const crypto::KeyPair& keyPair);

	private:
		x509_st* get();

	private:
		std::shared_ptr<x509_st> m_pCertificate;
	};

	// endregion
}}}
