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
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace test {

	/// Container of tests for sign and verify.
	template<typename TTraits>
	class SignVerifyTests {
	private:
		using KeyPair = typename TTraits::KeyPair;
		using Signature = typename TTraits::Signature;

	private:
		// region test utils

		template<typename TArray>
		static Signature SignPayload(const KeyPair& keyPair, const TArray& payload) {
			Signature signature;
			EXPECT_NO_THROW(Sign(keyPair, payload, signature));
			return signature;
		}

		// endregion

	public:
		// region Sign

		static void AssertSignFillsTheSignature() {
			// Arrange:
			auto payload = GenerateRandomArray<100>();

			// Act:
			Signature signature;
			std::iota(signature.begin(), signature.end(), static_cast<uint8_t>(0));
			Sign(TTraits::GenerateKeyPair(), payload, signature);

			// Assert: the signature got overwritten in call to Sign() above
			Signature invalid;
			std::iota(invalid.begin(), invalid.end(), static_cast<uint8_t>(0));
			EXPECT_NE(invalid, signature);
		}

		static void AssertSignaturesGeneratedForSameDataBySameKeyPairsAreEqual() {
			// Arrange:
			auto keyPair1 = TTraits::GenerateKeyPair();
			auto keyPair2 = KeyPair::FromPrivate(KeyPair::PrivateKey::FromBuffer(keyPair1.privateKey()));
			auto payload = GenerateRandomArray<100>();

			// Act:
			auto signature1 = SignPayload(keyPair1, payload);
			auto signature2 = SignPayload(keyPair2, payload);

			// Assert:
			EXPECT_EQ(signature1, signature2);
		}

		static void AssertSignaturesGeneratedForSameDataByDifferentKeyPairsAreDifferent() {
			// Arrange:
			auto payload = GenerateRandomArray<100>();

			// Act:
			auto signature1 = SignPayload(TTraits::GenerateKeyPair(), payload);
			auto signature2 = SignPayload(TTraits::GenerateKeyPair(), payload);

			// Assert:
			EXPECT_NE(signature1, signature2);
		}

		// endregion

		// region Verify

		static void AssertSignedDataCanBeVerified() {
			// Arrange:
			auto payload = GenerateRandomArray<100>();
			auto keyPair = TTraits::GenerateKeyPair();
			auto signature = SignPayload(keyPair, payload);

			// Act:
			bool isVerified = crypto::Verify(keyPair.publicKey(), payload, signature);

			// Assert:
			EXPECT_TRUE(isVerified);
		}

		static void AssertSignedDataCannotBeVerifiedWithDifferentKeyPair() {
			// Arrange:
			auto payload = GenerateRandomArray<100>();
			auto signature = SignPayload(TTraits::GenerateKeyPair(), payload);

			// Act:
			bool isVerified = crypto::Verify(TTraits::GenerateKeyPair().publicKey(), payload, signature);

			// Assert:
			EXPECT_FALSE(isVerified);
		}

	private:
		static void AssertSignatureChangeInvalidatesSignature(size_t position) {
			// Arrange:
			auto keyPair = TTraits::GenerateKeyPair();
			auto payload = GenerateRandomArray<100>();

			auto signature = SignPayload(keyPair, payload);
			signature[position] ^= 0xFF;

			// Act:
			bool isVerified = crypto::Verify(keyPair.publicKey(), payload, signature);

			// Assert:
			EXPECT_FALSE(isVerified);
		}

	public:
		static void AssertSignatureDoesNotVerifyWhenRPartOfSignatureIsModified() {
			for (auto i = 0u; i < Signature::Size / 2; ++i)
				AssertSignatureChangeInvalidatesSignature(i);
		}

		static void AssertSignatureDoesNotVerifyWhenSPartOfSignatureIsModified() {
			for (auto i = Signature::Size / 2; i < Signature::Size; ++i)
				AssertSignatureChangeInvalidatesSignature(i);
		}

		static void AssertSignatureDoesNotVerifyWhenPayloadIsModified() {
			// Arrange:
			auto keyPair = TTraits::GenerateKeyPair();
			auto payload = GenerateRandomArray<100>();
			for (auto i = 0u; i < payload.size(); ++i) {
				auto signature = SignPayload(keyPair, payload);
				payload[i] ^= 0xFF;

				// Act:
				bool isVerified = crypto::Verify(keyPair.publicKey(), payload, signature);

				// Assert:
				EXPECT_FALSE(isVerified);
			}
		}

		static void AssertPublicKeyNotOnACurveCausesVerifyToFail() {
			// Arrange:
			auto hackedKeyPair = TTraits::GenerateKeyPair();
			auto payload = GenerateRandomArray<100>();

			// hack the key, to an invalid one (not on a curve)
			auto& hackPublic = const_cast<typename KeyPair::PublicKey&>(hackedKeyPair.publicKey());
			std::fill(hackPublic.begin(), hackPublic.end(), static_cast<uint8_t>(0));
			hackPublic[hackPublic.size() - 1] = 0x01;

			auto signature = SignPayload(hackedKeyPair, payload);

			// Act:
			bool isVerified = crypto::Verify(hackedKeyPair.publicKey(), payload, signature);

			// Assert:
			EXPECT_FALSE(isVerified);
		}

		static void AssertVerificationFailsWhenPublicKeyDoesNotCorrespondToPrivateKey() {
			// Arrange:
			auto hackedKeyPair = TTraits::GenerateKeyPair();
			auto payload = GenerateRandomArray<100>();

			// hack the key, to an invalid one
			auto& hackPublic = const_cast<typename KeyPair::PublicKey&>(hackedKeyPair.publicKey());
			std::transform(hackPublic.begin(), hackPublic.end(), hackPublic.begin(), [](uint8_t x) {
				return static_cast<uint8_t>(x ^ 0xFF);
			});

			auto signature = SignPayload(hackedKeyPair, payload);

			// Act:
			bool isVerified = crypto::Verify(hackedKeyPair.publicKey(), payload, signature);

			// Assert:
			EXPECT_FALSE(isVerified);
		}

		static void AssertVerifyRejectsZeroPublicKey() {
			// Arrange:
			auto hackedKeyPair = TTraits::GenerateKeyPair();
			auto payload = GenerateRandomArray<100>();

			// hack the key, to an invalid one
			auto& hackPublic = const_cast<typename KeyPair::PublicKey&>(hackedKeyPair.publicKey());
			std::fill(hackPublic.begin(), hackPublic.end(), static_cast<uint8_t>(0));

			auto signature = SignPayload(hackedKeyPair, payload);

			// Act:
			// keep in mind, there's no good way to make this test, as right now, we have
			// no way (and I don't think we need one), to check why verify failed
			bool isVerified = crypto::Verify(hackedKeyPair.publicKey(), payload, signature);

			// Assert:
			EXPECT_FALSE(isVerified);
		}

		static void AssertCannotVerifyNonCanonicalSignature() {
			// Arrange:
			auto payload = TTraits::GetPayloadForNonCanonicalSignatureTest();

			auto keyPair = TTraits::GenerateKeyPair();
			auto canonicalSignature = SignPayload(keyPair, payload);
			auto nonCanonicalSignature = TTraits::MakeNonCanonical(canonicalSignature);

			// Act:
			bool isCanonicalVerified = crypto::Verify(keyPair.publicKey(), payload, canonicalSignature);
			bool isNonCanonicalVerified = crypto::Verify(keyPair.publicKey(), payload, nonCanonicalSignature);

			// Assert:
			EXPECT_TRUE(isCanonicalVerified);
			EXPECT_FALSE(isNonCanonicalVerified);
		}

		// endregion

		// region sign chunked data

		static void AssertSignatureForConsecutiveDataMatchesSignatureForChunkedData() {
			// Arrange:
			auto payload = GenerateRandomVector(123);
			auto keyPair = TTraits::GenerateKeyPair();
			auto properSignature = SignPayload(keyPair, payload);

			// Act:
			{
				Signature result;
				auto partSize = payload.size() / 2;
				ASSERT_NO_THROW(Sign(keyPair, {
					{ payload.data(), partSize },
					{ payload.data() + partSize, payload.size() - partSize }
				}, result));
				EXPECT_EQ(properSignature, result);
			}

			{
				Signature result;
				auto partSize = payload.size() / 3;
				ASSERT_NO_THROW(Sign(keyPair, {
					{ payload.data(), partSize },
					{ payload.data() + partSize, partSize },
					{ payload.data() + 2 * partSize, payload.size() - 2 * partSize }
				}, result));
				EXPECT_EQ(properSignature, result);
			}

			{
				Signature result;
				auto partSize = payload.size() / 4;
				ASSERT_NO_THROW(Sign(keyPair, {
					{ payload.data(), partSize },
					{ payload.data() + partSize, partSize },
					{ payload.data() + 2 * partSize, partSize },
					{ payload.data() + 3 * partSize, payload.size() - 3 * partSize }
				}, result));
				EXPECT_EQ(properSignature, result);
			}
		}

		// endregion
	};

#define MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::SignVerifyTests<TRAITS_NAME>::Assert##TEST_NAME(); }

/// Adds all sign and verify tests for the specified traits (\a TRAITS_NAME).
#define DEFINE_SIGN_VERIFY_TESTS(TRAITS_NAME) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignFillsTheSignature) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignaturesGeneratedForSameDataBySameKeyPairsAreEqual) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignaturesGeneratedForSameDataByDifferentKeyPairsAreDifferent) \
	\
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignedDataCanBeVerified) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignedDataCannotBeVerifiedWithDifferentKeyPair) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignatureDoesNotVerifyWhenRPartOfSignatureIsModified) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignatureDoesNotVerifyWhenSPartOfSignatureIsModified) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignatureDoesNotVerifyWhenPayloadIsModified) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, PublicKeyNotOnACurveCausesVerifyToFail) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, VerificationFailsWhenPublicKeyDoesNotCorrespondToPrivateKey) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, VerifyRejectsZeroPublicKey) \
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, CannotVerifyNonCanonicalSignature) \
	\
	MAKE_SIGN_VERIFY_TEST(TRAITS_NAME, SignatureForConsecutiveDataMatchesSignatureForChunkedData)
}}
