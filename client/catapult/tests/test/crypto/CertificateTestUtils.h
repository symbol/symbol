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
struct x509_store_ctx_st;

namespace catapult { namespace test {

	// region key utils

	/// Generates a random certificate key.
	std::shared_ptr<evp_pkey_st> GenerateRandomCertificateKey();

	/// Generates a certificate key from \a keyPair.
	std::shared_ptr<evp_pkey_st> GenerateCertificateKey(const crypto::KeyPair& keyPair);

	// endregion

	// region certificate store context utils

	/// Custom deleter for X509 structures.
	struct CertificateDeleter {
		/// Deletes \a pCertificate.
		void operator()(x509_st* pCertificate) const;
	};

	/// Unique pointer for X509 structures.
	using CertificatePointer = std::unique_ptr<x509_st, CertificateDeleter>;

	/// Holds a certificate store context and certificates.
	struct CertificateStoreContextHolder {
		/// Certificates.
		std::vector<x509_st*> Certificates;

		/// Certificate store context.
		std::shared_ptr<x509_store_ctx_st> pCertificateStoreContext;
	};

	/// Sets the active certificate in \a holder to the certificate at \a index.
	void SetActiveCertificate(CertificateStoreContextHolder& holder, size_t index);

	/// Creates a certificate store context around \a certificates.
	CertificateStoreContextHolder CreateCertificateStoreContextFromCertificates(std::vector<CertificatePointer>&& certificates);

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

		/// Sets the public key to \a key.
		void setPublicKey(evp_pkey_st& key);

	public:
		/// Builds the certificate.
		CertificatePointer build();

		/// Builds and signs the certificate with previously specified key.
		CertificatePointer buildAndSign();

		/// Builds and signs the certificate with \a key.
		CertificatePointer buildAndSign(evp_pkey_st& key);

	private:
		x509_st* get();

	private:
		evp_pkey_st* m_pKey;
		CertificatePointer m_pCertificate;
	};

	// endregion

	// region PemCertificate

	/// Pem certificate generator.
	class PemCertificate {
	public:
		/// Creates pem certificate with random node key pair.
		PemCertificate();

		/// Creates pem certificate around \a caKeyPair and \a nodeKeyPair.
		PemCertificate(const crypto::KeyPair& caKeyPair, const crypto::KeyPair& nodeKeyPair);

	public:
		/// Gets the CA public key in pem format.
		const std::string& caPublicKeyString() const;

		/// Gets the node private key in pem format.
		const std::string& nodePrivateKeyString() const;

		/// Gets the certificate chain in pem format.
		const std::string& certificateChainString() const;

	private:
		std::string m_caPublicKey;
		std::string m_nodePrivateKey;
		std::string m_pemCertificateChain;
	};

	// endregion
}}
