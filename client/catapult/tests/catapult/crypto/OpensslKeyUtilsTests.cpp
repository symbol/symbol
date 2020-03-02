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

#include "catapult/crypto/OpensslKeyUtils.h"
#include "tests/test/crypto/CertificateTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"
#include <fstream>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/pem.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

#define TEST_CLASS OpensslKeyUtilsTests

	// region traits

	namespace {
		struct PublicKeyTraits {
			static constexpr auto Read = ReadPublicKeyFromPrivateKeyPemFile;

			static void AssertReadResult(const KeyPair& expectedKeyPair, const Key& actualPublicKey) {
				EXPECT_EQ(expectedKeyPair.publicKey(), actualPublicKey);
			}
		};

		struct KeyPairTraits {
			static constexpr auto Read = ReadKeyPairFromPrivateKeyPemFile;

			static void AssertReadResult(const KeyPair& expectedKeyPair, const KeyPair& actualKeyPair) {
				EXPECT_EQ(expectedKeyPair.publicKey(), actualKeyPair.publicKey());
				EXPECT_EQ(expectedKeyPair.privateKey(), actualKeyPair.privateKey());
			}
		};
	}

#define KEY_TYPE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_PublicKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PublicKeyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_KeyPair) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<KeyPairTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	namespace {
		void WritePrivateKeyPemFile(const KeyPair& keyPair, const std::string& filename) {
			auto pOut = std::shared_ptr<BIO>(BIO_new_file(filename.c_str(), "w"), BIO_free);
			if (!pOut)
				throw std::bad_alloc();

			auto pKey = test::GenerateCertificateKey(keyPair);
			if (!PEM_write_bio_PrivateKey(pOut.get(), pKey.get(), nullptr, nullptr, 0, nullptr, nullptr))
				CATAPULT_THROW_RUNTIME_ERROR("error writing key to file");
		}
	}

	KEY_TYPE_TRAITS_BASED_TEST(ReadFailsWhenFileDoesNotExist) {
		// Arrange:
		test::TempFileGuard fileGuard("key.pem");

		// Act + Assert:
		EXPECT_THROW(TTraits::Read(fileGuard.name()), catapult_invalid_argument);
	}

	KEY_TYPE_TRAITS_BASED_TEST(ReadFailsWhenFileIsCorrupt) {
		// Arrange:
		test::TempFileGuard fileGuard("key.pem");
		{
			std::ofstream fout(fileGuard.name(), std::ios_base::out);
			fout << test::GenerateRandomByteArray<Key>();
		}

		// Act + Assert:
		EXPECT_THROW(TTraits::Read(fileGuard.name()), catapult_invalid_argument);
	}

	KEY_TYPE_TRAITS_BASED_TEST(ReadSucceedsWhenFileIsValid) {
		// Arrange:
		test::TempFileGuard fileGuard("key.pem");
		auto keyPair = test::GenerateKeyPair();
		WritePrivateKeyPemFile(keyPair, fileGuard.name());

		// Act:
		auto readResult = TTraits::Read(fileGuard.name());

		// Assert:
		TTraits::AssertReadResult(keyPair, readResult);
	}
}}
