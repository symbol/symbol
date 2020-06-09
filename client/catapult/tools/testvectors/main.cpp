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

#include "tools/ToolMain.h"
#include "catapult/crypto/AesCbcDecrypt.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/SharedKey.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/Address.h"
#include "catapult/model/MosaicIdGenerator.h"
#include "catapult/utils/HexParser.h"
#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif
#include <boost/filesystem/path.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

namespace catapult { namespace tools { namespace testvectors {

	namespace {
		// region helpers

		template<typename T = std::string>
		auto Get(const pt::ptree& tree, const std::string& key) {
			auto value = tree.get_optional<T>(key);
			if (!value.is_initialized()) {
				std::ostringstream message;
				message << "required property '" << key << "' was not found in json";
				CATAPULT_THROW_RUNTIME_ERROR(message.str().c_str());
			}

			return value.get();
		}

		// endregion

		// region data parsing

		void AssertSize(const std::string& name, size_t expectedSize, const RawString& str, size_t testCaseNumber) {
			if (expectedSize != str.Size) {
				std::ostringstream out;
				out
						<< "invalid data " << name << " in test case " << testCaseNumber << " has invalid size: "
						<< str.Size << ", expected: " << expectedSize;
				CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
			}
		}

		auto ParseVector(const RawString& str, size_t testCaseNumber, size_t expectedLength) {
			AssertSize("data", 2 * expectedLength, str, testCaseNumber);

			std::vector<uint8_t> buffer(str.Size / 2);
			utils::ParseHexStringIntoContainer(str.pData, str.Size, buffer);
			return buffer;
		}

		template<typename TByteArray>
		auto ParseByteArray(const std::string& name, const RawString& str, size_t testCaseNumber) {
			AssertSize(name, 2 * TByteArray::Size, str, testCaseNumber);
			return utils::ParseByteArray<TByteArray>(std::string(str.pData, 2 * TByteArray::Size));
		}

		auto ParseHash256(const RawString& str, size_t testCaseNumber) {
			return ParseByteArray<Hash256>("hash", str, testCaseNumber);
		}

		auto ParsePublicKey(const RawString& str, size_t testCaseNumber) {
			return ParseByteArray<Key>("public key", str, testCaseNumber);
		}

		auto ParseSignature(const RawString& str, size_t testCaseNumber) {
			return ParseByteArray<Signature>("signature", str, testCaseNumber);
		}

		auto ParsePrivateKey(const RawString& str, size_t testCaseNumber) {
			AssertSize("private key", 2 * Key::Size, str, testCaseNumber);
			return crypto::PrivateKey::FromString(std::string(str.pData, 2 * Key::Size));
		}

		auto ParseAddress(const RawString& str, size_t testCaseNumber) {
			constexpr size_t Address_Encoded_Size = 39;
			AssertSize("address", Address_Encoded_Size, str, testCaseNumber);
			return model::StringToAddress(std::string(str.pData, str.Size));
		}

		auto ParseMosaicId(const RawString& str, size_t testCaseNumber) {
			AssertSize("mosaic id", 2 * sizeof(MosaicId::ValueType), str, testCaseNumber);
			auto buffer = ParseVector(str, testCaseNumber, sizeof(MosaicId::ValueType));

			MosaicId::ValueType rawMosaicId = 0;
			for (auto byte : buffer) {
				rawMosaicId <<= 8;
				rawMosaicId += byte;
			}

			return MosaicId(rawMosaicId);
		}

		auto ParseSharedKey(const RawString& str, size_t testCaseNumber) {
			return ParseByteArray<crypto::SharedKey>("sharedKey", str, testCaseNumber);
		}

		auto ParseAesInitializationVector(const RawString& str, size_t testCaseNumber) {
			return ParseByteArray<crypto::AesInitializationVector>("iv (initialization vector)", str, testCaseNumber);
		}

		// endregion

		auto CreateHashTester(const consumer<const RawBuffer&, Hash256&>& hashFunc) {
			return [hashFunc] (const auto& testCase, auto testCaseNumber) {
				// Arrange:
				auto expectedHash = ParseHash256(Get<>(testCase, "hash"), testCaseNumber);
				auto length = Get<size_t>(testCase, "length");
				auto buffer = ParseVector(Get<>(testCase, "data"), testCaseNumber, length);

				// Act:
				Hash256 result;
				hashFunc(buffer, result);

				// Assert:
				return expectedHash == result;
			};
		}

		bool KeyConversionTester(const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto privateKey = ParsePrivateKey(Get<>(testCase, "privateKey"), testCaseNumber);
			auto expectedPublicKey = ParsePublicKey(Get<>(testCase, "publicKey"), testCaseNumber);

			// Act:
			auto keyPair = crypto::KeyPair::FromPrivate(std::move(privateKey));

			// Assert:
			return expectedPublicKey == keyPair.publicKey();
		}

		bool AddressConversionTester(const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto publicKey = ParsePublicKey(Get<>(testCase, "publicKey"), testCaseNumber);
			auto expectedAddressPublic = ParseAddress(Get<>(testCase, "address_Public"), testCaseNumber);
			auto expectedAddressPublicTest = ParseAddress(Get<>(testCase, "address_PublicTest"), testCaseNumber);
			auto expectedAddressMijin = ParseAddress(Get<>(testCase, "address_Mijin"), testCaseNumber);
			auto expectedAddressMijinTest = ParseAddress(Get<>(testCase, "address_MijinTest"), testCaseNumber);

			// Act:
			auto addressPublic = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Public);
			auto addressPublicTest = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Public_Test);
			auto addressMijin = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Mijin);
			auto addressMijinTest = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Mijin_Test);

			// Assert:
			return expectedAddressPublic == addressPublic &&
					expectedAddressPublicTest == addressPublicTest &&
					expectedAddressMijin == addressMijin &&
					expectedAddressMijinTest == addressMijinTest;
		}

		bool SigningTester(const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto privateKey = ParsePrivateKey(Get<>(testCase, "privateKey"), testCaseNumber);
			auto publicKey = ParsePublicKey(Get<>(testCase, "publicKey"), testCaseNumber);
			auto keyPair = crypto::KeyPair::FromPrivate(std::move(privateKey));
			auto length = Get<size_t>(testCase, "length");
			auto buffer = ParseVector(Get<>(testCase, "data"), testCaseNumber, length);
			auto expectedSignature = ParseSignature(Get<>(testCase, "signature"), testCaseNumber);

			// Sanity:
			if (publicKey != keyPair.publicKey())
				CATAPULT_THROW_RUNTIME_ERROR_1("public key mismatch", testCaseNumber);

			// Act:
			Signature signature;
			crypto::Sign(keyPair, buffer, signature);

			// Assert:
			return expectedSignature == signature;
		}

		bool DeriveTester(const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto privateKey = ParsePrivateKey(Get<>(testCase, "privateKey"), testCaseNumber);
			auto otherPublicKey = ParsePublicKey(Get<>(testCase, "otherPublicKey"), testCaseNumber);
			auto keyPair = crypto::KeyPair::FromPrivate(std::move(privateKey));
			auto expectedSharedKey = ParseSharedKey(Get<>(testCase, "sharedKey"), testCaseNumber);

			// Act:
			auto sharedKey = crypto::DeriveSharedKey(keyPair, otherPublicKey);

			// Assert:
			return expectedSharedKey == sharedKey;
		}

		auto Concatenate(const crypto::AesInitializationVector& initializationVector, const std::vector<uint8_t>& encryptedPayload) {
			std::vector<uint8_t> result;
			result.resize(initializationVector.size() + encryptedPayload.size());

			std::memcpy(result.data(), initializationVector.data(), initializationVector.size());
			std::memcpy(result.data() + initializationVector.size(), encryptedPayload.data(), encryptedPayload.size());

			return result;
		}

		bool DecryptTester(const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto privateKey = ParsePrivateKey(Get<>(testCase, "privateKey"), testCaseNumber);
			auto otherPublicKey = ParsePublicKey(Get<>(testCase, "otherPublicKey"), testCaseNumber);
			auto keyPair = crypto::KeyPair::FromPrivate(std::move(privateKey));
			auto sharedKey = crypto::DeriveSharedKey(keyPair, otherPublicKey);

			auto iv = ParseAesInitializationVector(Get<>(testCase, "iv"), testCaseNumber);
			auto cipherTextStr = Get<>(testCase, "cipherText");
			auto cipherText = ParseVector(cipherTextStr, testCaseNumber, cipherTextStr.size() / 2);
			auto clearTextStr = Get<>(testCase, "clearText");
			auto clearText = ParseVector(clearTextStr, testCaseNumber, clearTextStr.size() / 2);

			auto encrypted = Concatenate(iv, cipherText);

			// Act:
			std::vector<uint8_t> decrypted;
			auto decryptionResult = TryAesCbcDecrypt(sharedKey, encrypted, decrypted);

			// Assert:
			return decryptionResult && clearText == decrypted;
		}

		bool MosaicIdDerivationTester(const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto nonce = MosaicNonce(Get<MosaicNonce::ValueType>(testCase, "mosaicNonce"));
			auto addressPublic = ParseAddress(Get<>(testCase, "address_Public"), testCaseNumber);
			auto addressPublicTest = ParseAddress(Get<>(testCase, "address_PublicTest"), testCaseNumber);
			auto addressMijin = ParseAddress(Get<>(testCase, "address_Mijin"), testCaseNumber);
			auto addressMijinTest = ParseAddress(Get<>(testCase, "address_MijinTest"), testCaseNumber);

			auto expectedMosaicIdPublic = ParseMosaicId(Get<>(testCase, "mosaicId_Public"), testCaseNumber);
			auto expectedMosaicIdPublicTest = ParseMosaicId(Get<>(testCase, "mosaicId_PublicTest"), testCaseNumber);
			auto expectedMosaicIdMijin = ParseMosaicId(Get<>(testCase, "mosaicId_Mijin"), testCaseNumber);
			auto expectedMosaicIdMijinTest = ParseMosaicId(Get<>(testCase, "mosaicId_MijinTest"), testCaseNumber);

			// Act:
			auto mosaicIdPublic = model::GenerateMosaicId(addressPublic, nonce);
			auto mosaicIdPublicTest = model::GenerateMosaicId(addressPublicTest, nonce);
			auto mosaicIdMijin = model::GenerateMosaicId(addressMijin, nonce);
			auto mosaicIdMijinTest = model::GenerateMosaicId(addressMijinTest, nonce);

			// Assert:
			return expectedMosaicIdPublic == mosaicIdPublic &&
					expectedMosaicIdPublicTest == mosaicIdPublicTest &&
					expectedMosaicIdMijin == mosaicIdMijin &&
					expectedMosaicIdMijinTest == mosaicIdMijinTest;
		}

		class TestVectorsTool : public Tool {
		public:
			std::string name() const override {
				return "Test-vectors Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional& positional) override {
				optionsBuilder("vectors-dir,v",
						OptionsValue<std::string>(m_vectorsDirectory)->required(),
						"path to test-vectors directory");

				positional.add("vectors-dir", -1);
			}

			int run(const Options&) override {
				RunTest(parseJsonFile("0.test-sha3-256"), "sha3", CreateHashTester(crypto::Sha3_256));
				RunTest(parseJsonFile("1.test-keys"), "key conversion", KeyConversionTester);
				RunTest(parseJsonFile("1.test-address"), "address conversion", AddressConversionTester);
				RunTest(parseJsonFile("2.test-sign"), "signing", SigningTester);
				RunTest(parseJsonFile("3.test-derive"), "shared key derive", DeriveTester);
				RunTest(parseJsonFile("4.test-cipher"), "aes-cbc decryption", DecryptTester);
				RunTest(parseJsonFile("5.test-mosaic-id"), "mosaic id derivation", MosaicIdDerivationTester);
				return 0;
			}

		private:
			pt::ptree parseJsonFile(const std::string& filename) {
				auto path = boost::filesystem::path(m_vectorsDirectory) / (filename + ".json");
				pt::ptree testData;
				pt::read_json(path.generic_string(), testData);
				return testData;
			}

		private:
			static void RunTest(pt::ptree&& testCases, const std::string& testName, const predicate<const pt::ptree&, size_t>& testFunc) {
				size_t testCaseNumber = 0;
				size_t numFailed = 0;

				for (const auto& testCasePair : testCases) {
					if (!testFunc(testCasePair.second, testCaseNumber))
						++numFailed;

					++testCaseNumber;
				}

				if (numFailed)
					CATAPULT_LOG(warning) << testName << " test: " << numFailed << " failures out of " << testCaseNumber;
				else
					CATAPULT_LOG(info) << testName << " test: " << testCaseNumber << " successes";
			}

		private:
			std::string m_vectorsDirectory;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::testvectors::TestVectorsTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
