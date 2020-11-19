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

#include "catapult/ionet/PacketSocketOptions.h"
#include "tests/test/crypto/CertificateTestUtils.h"
#include "tests/test/net/CertificateLocator.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"
#include <boost/asio/ssl.hpp>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/x509.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace ionet {

#define TEST_CLASS PacketSocketOptionsTests

	// region PacketSocketSslVerifyContext

	TEST(TEST_CLASS, PacketSocketSslVerifyContext_CanCreateDefault) {
		// Act:
		PacketSocketSslVerifyContext context;

		// Assert:
		EXPECT_FALSE(context.preverified());
		EXPECT_NO_THROW(context.publicKey());
	}

	TEST(TEST_CLASS, PacketSocketSslVerifyContext_CanCreateWithArguments) {
		// Arrange:
		boost::asio::ssl::verify_context asioVerifyContext(nullptr);
		Key publicKey;

		// Act:
		PacketSocketSslVerifyContext context(true, asioVerifyContext, publicKey);

		// Assert:
		EXPECT_TRUE(context.preverified());
		EXPECT_EQ(&asioVerifyContext, &context.asioVerifyContext());
		EXPECT_EQ(&publicKey, &context.publicKey());
	}

	TEST(TEST_CLASS, PacketSocketSslVerifyContext_CanChangePublicKey) {
		// Arrange:
		boost::asio::ssl::verify_context asioVerifyContext(nullptr);
		Key publicKey;
		PacketSocketSslVerifyContext context(true, asioVerifyContext, publicKey);

		// Act:
		auto publicKey2 = test::GenerateRandomByteArray<Key>();
		context.setPublicKey(publicKey2);

		// Assert:
		EXPECT_EQ(publicKey2, publicKey);
	}

	// endregion

	// region CreateSslContextSupplier

	TEST(TEST_CLASS, CreateSslContextSupplier_CanCreateSslContext) {
		// Arrange:
		auto supplier = CreateSslContextSupplier(test::GetDefaultCertificateDirectory());

		// Act + Assert:
		EXPECT_NO_THROW(supplier());
	}

	TEST(TEST_CLASS, CreateSslContextSupplier_SslContextIsSingleton) {
		// Arrange:
		auto supplier = CreateSslContextSupplier(test::GetDefaultCertificateDirectory());

		// Act:
		auto& sslContext1 = supplier();
		auto& sslContext2 = supplier();

		// Assert:
		EXPECT_EQ(&sslContext1, &sslContext2);
	}

	// endregion

	// region CreateSslVerifyCallbackSupplier

	namespace {
		auto CreateChainedCertificateStoreContext(const crypto::KeyPair& rootKeyPair, bool useValidSelfSignature) {
			auto pCertificateKey = test::GenerateCertificateKey(rootKeyPair);

			// root certificate - self-sign
			test::CertificateBuilder builder1;
			builder1.setSubject("JP", "NEM", "Root");
			builder1.setIssuer("JP", "NEM", "Root");
			builder1.setPublicKey(*pCertificateKey);
			auto certificate1 = useValidSelfSignature ? builder1.buildAndSign(*pCertificateKey) : builder1.build();

			// node certificate - sign with root key pair
			test::CertificateBuilder builder2;
			builder2.setSubject("JP", "NEM", "Node");
			builder2.setIssuer("JP", "NEM", "Root");
			builder2.setPublicKey(*test::GenerateRandomCertificateKey());
			auto certificate2 = builder2.buildAndSign(*pCertificateKey);

			std::vector<test::CertificatePointer> certificates;
			certificates.push_back(std::move(certificate1));
			certificates.push_back(std::move(certificate2));
			auto holder = test::CreateCertificateStoreContextFromCertificates(std::move(certificates));
			X509_STORE_CTX_set_error(holder.pCertificateStoreContext.get(), X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN);
			return holder;
		}
	}

	TEST(TEST_CLASS, CreateSslVerifyCallbackSupplier_SuccessVerificationDelegatesToProcessor) {
		// Arrange:
		auto predicate = CreateSslVerifyCallbackSupplier()();

		auto keyPair = test::GenerateKeyPair();
		auto holder = CreateChainedCertificateStoreContext(keyPair, true);

		Key publicKey;
		boost::asio::ssl::verify_context asioVerifyContext(holder.pCertificateStoreContext.get());
		PacketSocketSslVerifyContext contextPreverifyFalse(false, asioVerifyContext, publicKey);
		PacketSocketSslVerifyContext contextPreverifyTrue(true, asioVerifyContext, publicKey);

		// Act: simulate two level chain  - root(false), root(true), node(true)
		auto result1 = predicate(contextPreverifyFalse);
		auto result2 = predicate(contextPreverifyTrue);

		test::SetActiveCertificate(holder, 1);
		auto result3 = predicate(contextPreverifyTrue);

		// Assert:
		EXPECT_TRUE(result1);
		EXPECT_TRUE(result2);
		EXPECT_TRUE(result3);
		EXPECT_EQ(keyPair.publicKey(), publicKey);
	}

	TEST(TEST_CLASS, CreateSslVerifyCallbackSupplier_FailureVerificationDelegatesToProcessor) {
		// Arrange:
		auto predicate = CreateSslVerifyCallbackSupplier()();

		auto keyPair = test::GenerateKeyPair();
		auto holder = CreateChainedCertificateStoreContext(keyPair, false);

		Key publicKey;
		boost::asio::ssl::verify_context asioVerifyContext(holder.pCertificateStoreContext.get());
		PacketSocketSslVerifyContext contextPreverifyFalse(false, asioVerifyContext, publicKey);

		// Act: simulate two level chain  - root(false); first failure short circuits processing of certificate chain
		auto result1 = predicate(contextPreverifyFalse);

		// Assert:
		EXPECT_FALSE(result1);
	}

	// endregion
}}
