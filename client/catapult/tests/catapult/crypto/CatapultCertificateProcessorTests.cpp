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

#include "catapult/crypto/CatapultCertificateProcessor.h"
#include "tests/test/crypto/CertificateTestUtils.h"
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

#define TEST_CLASS CatapultCertificateProcessorTests

	// region test utils

	namespace {
		int GetError(test::CertificateStoreContextHolder& holder) {
			return X509_STORE_CTX_get_error(holder.pCertificateStoreContext.get());
		}

		std::shared_ptr<X509> CreateDefaultCertificate(const std::string& commonName) {
			test::CertificateBuilder builder;
			builder.setSubject("JP", "NEM", commonName);
			builder.setIssuer("JP", "NEM", commonName);
			builder.setPublicKey(*test::GenerateRandomCertificateKey());
			return builder.buildAndSign();
		}

		test::CertificateStoreContextHolder CreateCertificateStoreContext(const std::vector<std::string>& commonNames) {
			std::vector<std::shared_ptr<X509>> certificates;
			for (const auto& commonName : commonNames)
				certificates.push_back(CreateDefaultCertificate(commonName));

			return test::CreateCertificateStoreContextFromCertificates(certificates);
		}
	}

	// endregion

	// region constructor

	TEST(TEST_CLASS, CanCreateProcessor) {
		// Act:
		CatapultCertificateProcessor processor;

		// Assert:
		EXPECT_EQ(0u, processor.size());
	}

	// endregion

	// region no active certificate

	TEST(TEST_CLASS, VerifyFailsWhenNoCertificateIsActive) {
		// Arrange:
		auto holder = CreateCertificateStoreContext({ "Alice", "Bob" });
		X509_STORE_CTX_set_current_cert(holder.pCertificateStoreContext.get(), nullptr);
		CatapultCertificateProcessor processor;

		// Act + Assert:
		EXPECT_THROW(processor.verify(true, *holder.pCertificateStoreContext), catapult_invalid_argument);
		EXPECT_EQ(0, GetError(holder));
	}

	// endregion

	// region preverified

	namespace {
		std::vector<bool> PreverifyMultiple(
				CatapultCertificateProcessor& processor,
				test::CertificateStoreContextHolder& holder,
				size_t count) {
			std::vector<bool> results;
			for (auto i = 0u; i < count; ++i) {
				test::SetActiveCertificate(holder, i);
				results.push_back(processor.verify(true, *holder.pCertificateStoreContext));
			}

			return results;
		}
	}

	TEST(TEST_CLASS, CanAddSinglePreverifiedCertificate) {
		// Arrange:
		auto holder = CreateCertificateStoreContext({ "Alice", "Bob" });
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResults = PreverifyMultiple(processor, holder, 1);

		// Assert:
		EXPECT_EQ(std::vector<bool>({ true }), verifyResults);
		EXPECT_EQ(0, GetError(holder));
		ASSERT_EQ(1u, processor.size());

		EXPECT_EQ("CN=Alice,O=NEM,C=JP", processor.certificate(0).Subject);
	}

	TEST(TEST_CLASS, CanAddTwoPreverifiedCertificates) {
		// Arrange:
		auto holder = CreateCertificateStoreContext({ "Alice", "Bob" });
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResults = PreverifyMultiple(processor, holder, 2);

		// Assert:
		EXPECT_EQ(std::vector<bool>({ true, true }), verifyResults);
		EXPECT_EQ(0, GetError(holder));
		ASSERT_EQ(2u, processor.size());

		EXPECT_EQ("CN=Alice,O=NEM,C=JP", processor.certificate(0).Subject);
		EXPECT_EQ("CN=Bob,O=NEM,C=JP", processor.certificate(1).Subject);
	}

	TEST(TEST_CLASS, CannotAddPreverifiedChainWithTooFewCertificates) {
		// Arrange:
		auto holder = CreateCertificateStoreContext({ "Alice" });
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResults = PreverifyMultiple(processor, holder, 1);

		// Assert:
		EXPECT_EQ(std::vector<bool>({ false }), verifyResults);
		EXPECT_EQ(0, GetError(holder));
		EXPECT_EQ(0u, processor.size());
	}

	TEST(TEST_CLASS, CannotAddPreverifiedChainWithTooManyCertificates) {
		// Arrange:
		auto holder = CreateCertificateStoreContext({ "Alice", "Bob", "Charlie" });
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResults = PreverifyMultiple(processor, holder, 3);

		// Assert:
		EXPECT_EQ(std::vector<bool>({ false, false, false }), verifyResults);
		EXPECT_EQ(0, GetError(holder));
		EXPECT_EQ(0u, processor.size());
	}

	TEST(TEST_CLASS, UnparseableCertificateTakesPrecedenceOverPreverifiedStatus) {
		// Arrange: create a certificate without a key
		test::CertificateBuilder builder;
		builder.setSubject("JP", "NEM", "Alice");
		builder.setIssuer("JP", "NEM", "Alice");
		auto holder = test::CreateCertificateStoreContextFromCertificates({ builder.build(), CreateDefaultCertificate("Bob") });
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResult = processor.verify(true, *holder.pCertificateStoreContext);

		// Assert:
		EXPECT_FALSE(verifyResult);
		EXPECT_EQ(X509_V_ERR_APPLICATION_VERIFICATION, GetError(holder));
		EXPECT_EQ(0u, processor.size());
	}

	TEST(TEST_CLASS, ConsecutiveCertificatesWithSamePublicKeyAreNotCollapsed) {
		// Arrange:
		auto pCertificateKey = test::GenerateRandomCertificateKey();

		test::CertificateBuilder builder1;
		builder1.setSubject("JP", "NEM", "Alice");
		builder1.setIssuer("JP", "NEM", "Alice");
		builder1.setPublicKey(*pCertificateKey);

		test::CertificateBuilder builder2;
		builder2.setSubject("CA", "SYM", "Bob");
		builder2.setIssuer("CA", "SYM", "Bob");
		builder2.setPublicKey(*pCertificateKey);

		auto holder = test::CreateCertificateStoreContextFromCertificates({ builder1.buildAndSign(), builder2.buildAndSign() });
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResults = PreverifyMultiple(processor, holder, 2);

		// Assert:
		EXPECT_EQ(std::vector<bool>({ true, true }), verifyResults);
		EXPECT_EQ(0, GetError(holder));
		ASSERT_EQ(2u, processor.size());

		EXPECT_EQ("CN=Alice,O=NEM,C=JP", processor.certificate(0).Subject);
		EXPECT_EQ("CN=Bob,O=SYM,C=CA", processor.certificate(1).Subject);
	}

	TEST(TEST_CLASS, ConsecutiveCertificatesWithSameSubjectAreNotCollapsed) {
		// Arrange:
		test::CertificateBuilder builder1;
		builder1.setSubject("JP", "NEM", "Alice");
		builder1.setIssuer("JP", "NEM", "Alice");
		builder1.setPublicKey(*test::GenerateRandomCertificateKey());

		test::CertificateBuilder builder2;
		builder2.setSubject("JP", "NEM", "Alice");
		builder2.setIssuer("JP", "NEM", "Alice");
		builder2.setPublicKey(*test::GenerateRandomCertificateKey());

		auto holder = test::CreateCertificateStoreContextFromCertificates({ builder1.buildAndSign(), builder2.buildAndSign() });
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResults = PreverifyMultiple(processor, holder, 2);

		// Assert:
		EXPECT_EQ(std::vector<bool>({ true, true }), verifyResults);
		EXPECT_EQ(0, GetError(holder));
		ASSERT_EQ(2u, processor.size());

		EXPECT_EQ("CN=Alice,O=NEM,C=JP", processor.certificate(0).Subject);
		EXPECT_EQ("CN=Alice,O=NEM,C=JP", processor.certificate(1).Subject);
	}

	// endregion

	// region not preverified

	TEST(TEST_CLASS, CanAddSelfSignedRootCertificate) {
		// Arrange:
		auto holder = CreateCertificateStoreContext({ "Alice", "Bob" });
		X509_STORE_CTX_set_error(holder.pCertificateStoreContext.get(), X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN);
		CatapultCertificateProcessor processor;

		// Act:
		auto verifyResult = processor.verify(false, *holder.pCertificateStoreContext);

		// Assert:
		EXPECT_TRUE(verifyResult);
		EXPECT_EQ(X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN, GetError(holder));
		ASSERT_EQ(0u, processor.size());
	}

	TEST(TEST_CLASS, CannotAddSelfSignedNonRootCertificate) {
		// Arrange:
		auto holder = CreateCertificateStoreContext({ "Alice", "Bob" });
		X509_STORE_CTX_set_error(holder.pCertificateStoreContext.get(), X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN);
		CatapultCertificateProcessor processor;

		auto verifyResult1 = processor.verify(false, *holder.pCertificateStoreContext);
		auto verifyResult2 = processor.verify(true, *holder.pCertificateStoreContext);
		test::SetActiveCertificate(holder, 1);

		// Act:
		auto verifyResult3 = processor.verify(false, *holder.pCertificateStoreContext);

		// Assert:
		EXPECT_TRUE(verifyResult1);
		EXPECT_TRUE(verifyResult2);
		EXPECT_FALSE(verifyResult3);
		EXPECT_EQ(X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN, GetError(holder));
		ASSERT_EQ(1u, processor.size());

		EXPECT_EQ("CN=Alice,O=NEM,C=JP", processor.certificate(0).Subject);
	}

	namespace {
		void AssertSelfSignedRootCertificateError(
				const std::string& subjectCommonName,
				const std::string& issuerCommonName,
				int errorCodeDelta,
				bool shouldSign) {
			// Arrange:
			test::CertificateBuilder builder;
			builder.setSubject("JP", "NEM", subjectCommonName);
			builder.setIssuer("JP", "NEM", issuerCommonName);
			builder.setPublicKey(*test::GenerateRandomCertificateKey());
			auto pCertificate = shouldSign ? builder.buildAndSign() : builder.build();

			auto holder = test::CreateCertificateStoreContextFromCertificates({ pCertificate, CreateDefaultCertificate("Bob") });
			X509_STORE_CTX_set_error(holder.pCertificateStoreContext.get(), X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN + errorCodeDelta);
			CatapultCertificateProcessor processor;

			// Act:
			auto verifyResult = processor.verify(false, *holder.pCertificateStoreContext);

			// Assert:
			EXPECT_FALSE(verifyResult);
			EXPECT_EQ(X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN + errorCodeDelta, GetError(holder));
			EXPECT_EQ(0u, processor.size());
		}
	}

	TEST(TEST_CLASS, CannotAddSelfSignedRootCertificate_UnexpectedError) {
		AssertSelfSignedRootCertificateError("Alice", "Alice", 1, true);
	}

	TEST(TEST_CLASS, CannotAddSelfSignedRootCertificate_WrongIssuer) {
		AssertSelfSignedRootCertificateError("Alice", "Bob", 0, true);
	}

	TEST(TEST_CLASS, CannotAddSelfSignedRootCertificate_Unsigned) {
		AssertSelfSignedRootCertificateError("Alice", "Alice", 0, false);
	}

	// endregion
}}
