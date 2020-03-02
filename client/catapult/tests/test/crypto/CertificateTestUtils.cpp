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

#include "CertificateTestUtils.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/utils/HexParser.h"
#include "catapult/exceptions.h"
#include "tests/test/nodeps/KeyTestUtils.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/bio.h>
#include <openssl/dh.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace test {

	// region key utils

	std::shared_ptr<EVP_PKEY> GenerateRandomCertificateKey() {
		return GenerateCertificateKey(GenerateKeyPair());
	}

	std::shared_ptr<EVP_PKEY> GenerateCertificateKey(const crypto::KeyPair& keyPair) {
		auto i = 0u;
		Key rawPrivateKey;
		for (auto byte : keyPair.privateKey())
			rawPrivateKey[i++] = byte;

		return std::shared_ptr<EVP_PKEY>(
				EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr, rawPrivateKey.data(), rawPrivateKey.size()),
				EVP_PKEY_free);
	}

	// endregion

	// region certificate store context utils

	void SetActiveCertificate(CertificateStoreContextHolder& holder, size_t index) {
		X509_STORE_CTX_set_current_cert(holder.pCertificateStoreContext.get(), holder.Certificates[index].get());
	}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

	namespace {
		struct CertificateStackDeleter {
			void operator()(STACK_OF(X509)* pStack) const {
				sk_X509_free(pStack);
			}
		};
	}

	CertificateStoreContextHolder CreateCertificateStoreContextFromCertificates(const std::vector<std::shared_ptr<X509>>& certificates) {
		CertificateStoreContextHolder holder;
		holder.Certificates = certificates;

		auto pCertificateStack = std::unique_ptr<STACK_OF(X509), CertificateStackDeleter>(sk_X509_new_null());
		for (const auto& pCertificate : holder.Certificates) {
			if (!pCertificateStack || !sk_X509_push(pCertificateStack.get(), pCertificate.get()))
				CATAPULT_THROW_RUNTIME_ERROR("failed to add certificate to stack");
		}

		holder.pCertificateStoreContext = std::shared_ptr<X509_STORE_CTX>(X509_STORE_CTX_new(), X509_STORE_CTX_free);
		if (!holder.pCertificateStoreContext || !X509_STORE_CTX_init(holder.pCertificateStoreContext.get(), nullptr, nullptr, nullptr))
			CATAPULT_THROW_RUNTIME_ERROR("failed to initialize certificate store context");

		X509_STORE_CTX_set0_verified_chain(holder.pCertificateStoreContext.get(), pCertificateStack.release());
		SetActiveCertificate(holder, 0);
		return holder;
	}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

	// endregion

	// region CertificateBuilder

	namespace {
		void AddTextEntry(X509_NAME& name, const std::string& key, const std::string& value) {
			if (!X509_NAME_add_entry_by_txt(&name, key.data(), MBSTRING_ASC, reinterpret_cast<const uint8_t*>(value.data()), -1, -1, 0))
				CATAPULT_THROW_RUNTIME_ERROR_1("failed to add text entry", key);
		}

		void AddTextEntries(X509_NAME& name, const std::string& country, const std::string& organization, const std::string& commonName) {
			AddTextEntry(name, "C", country);
			AddTextEntry(name, "O", organization);
			AddTextEntry(name, "CN", commonName);
		}
	}

	CertificateBuilder::CertificateBuilder()
			: m_pKey(nullptr)
			, m_pCertificate(std::shared_ptr<X509>(X509_new(), X509_free)) {
		if (!m_pCertificate)
			throw std::bad_alloc();

		// set the version
		if (!X509_set_version(get(), 0))
			CATAPULT_THROW_RUNTIME_ERROR("failed to set certificate version");

		// set the serial number
		if (!ASN1_INTEGER_set(X509_get_serialNumber(get()), 1))
			CATAPULT_THROW_RUNTIME_ERROR("failed to set certificate serial number");

		// set expiration from now until one year from now
		if (!X509_gmtime_adj(X509_getm_notBefore(get()), 0) || !X509_gmtime_adj(X509_getm_notAfter(get()), 31536000L))
			CATAPULT_THROW_RUNTIME_ERROR("failed to set certificate expiration");
	}

	void CertificateBuilder::setSubject(const std::string& country, const std::string& organization, const std::string& commonName) {
		AddTextEntries(*X509_get_subject_name(get()), country, organization, commonName);
	}

	void CertificateBuilder::setIssuer(const std::string& country, const std::string& organization, const std::string& commonName) {
		AddTextEntries(*X509_get_issuer_name(get()), country, organization, commonName);
	}

	void CertificateBuilder::setPublicKey(EVP_PKEY& key) {
		if (!X509_set_pubkey(get(), &key))
			CATAPULT_THROW_RUNTIME_ERROR("failed to set certificate public key");

		m_pKey = &key;
	}

	std::shared_ptr<X509> CertificateBuilder::build() {
		return m_pCertificate;
	}

	std::shared_ptr<X509> CertificateBuilder::buildAndSign() {
		return buildAndSign(*m_pKey);
	}

	std::shared_ptr<X509> CertificateBuilder::buildAndSign(EVP_PKEY& key) {
		if (0 == X509_sign(get(), &key, EVP_md_null()))
			CATAPULT_THROW_RUNTIME_ERROR("failed to sign certificate");

		return build();
	}

	X509* CertificateBuilder::get() {
		return m_pCertificate.get();
	}

	// endregion

	// region PemCertificate

	namespace {
		struct BioWrapper {
		public:
			BioWrapper() : m_pBio(std::shared_ptr<BIO>(BIO_new(BIO_s_mem()), BIO_free)) {
				if (!m_pBio)
					throw std::bad_alloc();
			}

		public:
			operator BIO*() const {
				return m_pBio.get();
			}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

			std::string toString() const {
				char* pBioData = nullptr;
				auto bioSize = static_cast<size_t>(BIO_get_mem_data(m_pBio.get(), &pBioData));
				return std::string(pBioData, pBioData + bioSize);
			}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

		private:
			std::shared_ptr<BIO> m_pBio;
		};

		auto GeneratePemKey(const crypto::KeyPair& keyPair) {
			BioWrapper bio;
			auto pKey = GenerateCertificateKey(keyPair);
			if (!PEM_write_bio_PrivateKey(bio, pKey.get(), nullptr, nullptr, 0, nullptr, nullptr))
				CATAPULT_THROW_RUNTIME_ERROR("error writing key to bio");

			return bio.toString();
		}

		auto GeneratePemCertificateChain(const crypto::KeyPair& caKeyPair, const crypto::KeyPair& nodeKeyPair) {
			auto pCaKey = GenerateCertificateKey(caKeyPair);
			CertificateBuilder caCertBuilder;
			caCertBuilder.setIssuer("XD", "CA", "Ca cert");
			caCertBuilder.setSubject("XD", "CA", "Ca cert");
			caCertBuilder.setPublicKey(*pCaKey.get());
			auto caCert = caCertBuilder.buildAndSign();

			auto pNodeKey = GenerateCertificateKey(nodeKeyPair);
			CertificateBuilder nodeCertBuilder;
			nodeCertBuilder.setIssuer("XD", "CA", "Ca cert");
			nodeCertBuilder.setSubject("US", "Symbol", "nijuichi");
			nodeCertBuilder.setPublicKey(*pNodeKey.get());
			auto nodeCert = nodeCertBuilder.buildAndSign(*pCaKey.get());

			// write both certs to bio (order matters)
			BioWrapper bio;
			if (!PEM_write_bio_X509(bio, nodeCert.get()))
				CATAPULT_THROW_RUNTIME_ERROR("error writing node cert to bio");

			if (!PEM_write_bio_X509(bio, caCert.get()))
				CATAPULT_THROW_RUNTIME_ERROR("error writing ca cert to bio");

			return bio.toString();
		}
	}

	PemCertificate::PemCertificate() : PemCertificate(GenerateKeyPair())
	{}

	PemCertificate::PemCertificate(const crypto::KeyPair& nodeKeyPair) : PemCertificate(GenerateKeyPair(), nodeKeyPair)
	{}

	PemCertificate::PemCertificate(const crypto::KeyPair& caKeyPair, const crypto::KeyPair& nodeKeyPair)
			: m_pemKey(GeneratePemKey(nodeKeyPair))
			, m_pemCertificateChain(GeneratePemCertificateChain(caKeyPair, nodeKeyPair))
	{}

	const std::string& PemCertificate::keyString() const {
		return m_pemKey;
	}

	const std::string& PemCertificate::certificateChainString() const {
		return m_pemCertificateChain;
	}

	// endregion
}}
