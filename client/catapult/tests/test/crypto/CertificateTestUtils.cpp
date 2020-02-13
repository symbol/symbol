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
#include "catapult/exceptions.h"
#include "tests/test/nodeps/KeyTestUtils.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
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
		if (!X509_set_version(get(), 2))
			CATAPULT_THROW_RUNTIME_ERROR("failed to set certificate version");

		// set the serial number
		if (!ASN1_INTEGER_set(X509_get_serialNumber(get()), 1))
			CATAPULT_THROW_RUNTIME_ERROR("failed to set certificate serial number");

		// set expiration from now until one year from now
		if (!X509_gmtime_adj(X509_get_notBefore(get()), 0) || !X509_gmtime_adj(X509_get_notAfter(get()), 31536000L))
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
}}

