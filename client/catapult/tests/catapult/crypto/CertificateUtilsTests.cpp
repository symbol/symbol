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

#include "catapult/crypto/CertificateUtils.h"
#include "tests/test/crypto/CertificateTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/x509.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

#define TEST_CLASS CertificateUtilsTests

	// region TryParseCertificate

	TEST(TEST_CLASS, TryParseCertificate_CanParseCertificateWithSubjectAndPublicKey) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		builder.setPublicKey(*test::GenerateCertificateKey(keyPair));
		auto pCertificate = builder.build();

		// Act:
		CertificateInfo certificateInfo;
		auto result = TryParseCertificate(*pCertificate, certificateInfo);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ("CN=Alice,O=NEM,C=JP", certificateInfo.Subject);
		EXPECT_EQ(keyPair.publicKey(), certificateInfo.PublicKey);
	}

	TEST(TEST_CLASS, TryParseCertificate_CanParseCertificateWithSubjectWithControlCharactersAndPublicKey) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Al\1ce");
		builder.setPublicKey(*test::GenerateCertificateKey(keyPair));
		auto pCertificate = builder.build();

		// Act:
		CertificateInfo certificateInfo;
		auto result = TryParseCertificate(*pCertificate, certificateInfo);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ("CN=Al\\01ce,O=NEM,C=JP", certificateInfo.Subject);
		EXPECT_EQ(keyPair.publicKey(), certificateInfo.PublicKey);
	}

	TEST(TEST_CLASS, TryParseCertificate_CanParseCertificateWithoutSubject) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		test::CertificateBuilder builder;
		builder.setPublicKey(*test::GenerateCertificateKey(keyPair));
		auto pCertificate = builder.build();

		// Act:
		CertificateInfo certificateInfo;
		auto result = TryParseCertificate(*pCertificate, certificateInfo);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ("", certificateInfo.Subject);
		EXPECT_EQ(keyPair.publicKey(), certificateInfo.PublicKey);
	}

	TEST(TEST_CLASS, TryParseCertificate_CanParseCertificateWithLongSubject) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		test::CertificateBuilder builder;
		builder.setSubject("JP", std::string(64, 'b'), std::string(64, 'c'));
		builder.setSubject("CA", std::string(64, 'e'), std::string(64, 'f'));
		builder.setPublicKey(*test::GenerateCertificateKey(keyPair));
		auto pCertificate = builder.build();

		// Act:
		CertificateInfo certificateInfo;
		auto result = TryParseCertificate(*pCertificate, certificateInfo);

		// Assert:
		EXPECT_TRUE(result);

		// - only partial subject is extracted
		std::ostringstream expectedSubjectStream;
		expectedSubjectStream
				<< "CN=" << std::string(64, 'f')
				<< ",O=" << std::string(64, 'e')
				<< ",C=CA"
				<< ",CN=" << std::string(64, 'c')
				<< ",O=" << std::string(64, 'b')
				<< ",C=JP";

		EXPECT_EQ(expectedSubjectStream.str(), certificateInfo.Subject);
		EXPECT_EQ(keyPair.publicKey(), certificateInfo.PublicKey);
	}

	TEST(TEST_CLASS, TryParseCertificate_CannotParseCertificateWithoutPublicKey) {
		// Arrange:
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		auto pCertificate = builder.build();

		// Act:
		CertificateInfo certificateInfo;
		auto result = TryParseCertificate(*pCertificate, certificateInfo);

		// Assert:
		EXPECT_FALSE(result);
		EXPECT_EQ("CN=Alice,O=NEM,C=JP", certificateInfo.Subject);
		EXPECT_EQ(Key(), certificateInfo.PublicKey);
	}

	namespace {
		std::shared_ptr<EVP_PKEY> GenerateCertificateKeyWrongType(const crypto::KeyPair& keyPair) {
			auto i = 0u;
			Key rawPrivateKey;
			for (auto byte : keyPair.privateKey())
				rawPrivateKey[i++] = byte;

			return std::shared_ptr<EVP_PKEY>(
					EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, nullptr, rawPrivateKey.data(), rawPrivateKey.size()),
					EVP_PKEY_free);
		}
	}

	TEST(TEST_CLASS, TryParseCertificate_CannotParseCertificateWithWrongPublicKeyType) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		builder.setPublicKey(*GenerateCertificateKeyWrongType(keyPair));
		auto pCertificate = builder.build();

		// Act:
		CertificateInfo certificateInfo;
		auto result = TryParseCertificate(*pCertificate, certificateInfo);

		// Assert:
		EXPECT_FALSE(result);
		EXPECT_EQ("CN=Alice,O=NEM,C=JP", certificateInfo.Subject);
		EXPECT_EQ(Key(), certificateInfo.PublicKey);
	}

	// endregion

	// region VerifySelfSigned

	TEST(TEST_CLASS, VerifySelfSigned_ReturnsTrueForProperlySignedCertificate) {
		// Arrange:
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		builder.setIssuer("JP", "NEM", "Alice");
		builder.setPublicKey(*test::GenerateRandomCertificateKey());
		auto pCertificate = builder.buildAndSign();

		// Act + Assert:
		EXPECT_TRUE(VerifySelfSigned(*pCertificate));
	}

	TEST(TEST_CLASS, VerifySelfSigned_ReturnsFalseForUnsignedCertificate) {
		// Arrange:
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		builder.setIssuer("JP", "NEM", "Alice");
		builder.setPublicKey(*test::GenerateRandomCertificateKey());
		auto pCertificate = builder.build();

		// Act + Assert:
		EXPECT_FALSE(VerifySelfSigned(*pCertificate));
	}

	TEST(TEST_CLASS, VerifySelfSigned_ReturnsFalseForCertificateWithWrongIssuer) {
		// Arrange:
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		builder.setIssuer("JP", "NEM", "Bob");
		builder.setPublicKey(*test::GenerateRandomCertificateKey());
		auto pCertificate = builder.buildAndSign();

		// Act + Assert:
		EXPECT_FALSE(VerifySelfSigned(*pCertificate));
	}

	TEST(TEST_CLASS, VerifySelfSigned_ReturnsFalseForCertificateWithExternalSigner) {
		// Arrange:
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		builder.setIssuer("JP", "NEM", "Alice");
		builder.setPublicKey(*test::GenerateRandomCertificateKey());
		auto pCertificate = builder.buildAndSign(*test::GenerateRandomCertificateKey());

		// Act + Assert:
		EXPECT_FALSE(VerifySelfSigned(*pCertificate));
	}

	// endregion
}}
