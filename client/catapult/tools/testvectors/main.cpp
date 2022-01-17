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

#include "tools/ToolMain.h"
#include "catapult/crypto/AesDecrypt.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/crypto/SharedKey.h"
#include "catapult/crypto/Signer.h"
#include "catapult/extensions/Bip32.h"
#include "catapult/extensions/Bip39.h"
#include "catapult/model/Address.h"
#include "catapult/model/MosaicIdGenerator.h"
#include "catapult/utils/HexParser.h"
#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>
#include <optional>

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

		template<typename T>
		auto GetVector(const pt::ptree& tree, const std::string& key) {
			std::vector<T> values;

			for (const auto& pair : tree.get_child(key))
				values.push_back(pair.second.get_value<T>());

			return values;
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

		auto ParseHash512(const RawString& str, size_t testCaseNumber) {
			return ParseByteArray<Hash512>("hash", str, testCaseNumber);
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

		// endregion

		auto CreateHashTester(const consumer<const RawBuffer&, Hash256&>& hashFunc) {
			return [hashFunc] (const auto&, const auto& testCase, auto testCaseNumber) {
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

		std::optional<bool> KeyConversionTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto privateKey = ParsePrivateKey(Get<>(testCase, "privateKey"), testCaseNumber);
			auto expectedPublicKey = ParsePublicKey(Get<>(testCase, "publicKey"), testCaseNumber);

			// Act:
			auto keyPair = crypto::KeyPair::FromPrivate(std::move(privateKey));

			// Assert:
			return expectedPublicKey == keyPair.publicKey();
		}

		std::optional<bool> AddressConversionTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto publicKey = ParsePublicKey(Get<>(testCase, "publicKey"), testCaseNumber);
			auto expectedAddressMainnet = ParseAddress(Get<>(testCase, "address_Public"), testCaseNumber);
			auto expectedAddressTestnet = ParseAddress(Get<>(testCase, "address_PublicTest"), testCaseNumber);

			// Act:
			auto addressMainnet = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Mainnet);
			auto addressTestnet = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Testnet);

			// Assert:
			return expectedAddressMainnet == addressMainnet && expectedAddressTestnet == addressTestnet;
		}

		std::optional<bool> SignTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
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

		std::optional<bool> VerifyTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto publicKey = ParsePublicKey(Get<>(testCase, "publicKey"), testCaseNumber);
			auto length = Get<size_t>(testCase, "length");
			auto buffer = ParseVector(Get<>(testCase, "data"), testCaseNumber, length);
			auto signature = ParseSignature(Get<>(testCase, "signature"), testCaseNumber);

			// Act:
			auto isVerified = crypto::Verify(publicKey, buffer, signature);

			// Assert:
			return isVerified;
		}

		std::optional<bool> DeriveTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
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

		auto Concatenate(
				const crypto::AesGcm256::Tag& tag,
				const crypto::AesGcm256::IV& iv,
				const std::vector<uint8_t>& encryptedPayload) {
			std::vector<uint8_t> result;
			result.resize(tag.size() + iv.size() + encryptedPayload.size());

			std::memcpy(result.data(), tag.data(), tag.size());
			std::memcpy(result.data() + tag.size(), iv.data(), iv.size());
			std::memcpy(result.data() + tag.size() + iv.size(), encryptedPayload.data(), encryptedPayload.size());
			return result;
		}

		std::optional<bool> DecryptTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto privateKey = ParsePrivateKey(Get<>(testCase, "privateKey"), testCaseNumber);
			auto otherPublicKey = ParsePublicKey(Get<>(testCase, "otherPublicKey"), testCaseNumber);
			auto keyPair = crypto::KeyPair::FromPrivate(std::move(privateKey));
			auto sharedKey = crypto::DeriveSharedKey(keyPair, otherPublicKey);

			auto tag = ParseByteArray<crypto::AesGcm256::Tag>("tag", Get<>(testCase, "tag"), testCaseNumber);
			auto iv = ParseByteArray<crypto::AesGcm256::IV>("iv", Get<>(testCase, "iv"), testCaseNumber);
			auto cipherTextStr = Get<>(testCase, "cipherText");
			auto cipherText = ParseVector(cipherTextStr, testCaseNumber, cipherTextStr.size() / 2);
			auto clearTextStr = Get<>(testCase, "clearText");
			auto clearText = ParseVector(clearTextStr, testCaseNumber, clearTextStr.size() / 2);

			auto encrypted = Concatenate(tag, iv, cipherText);

			// Act:
			std::vector<uint8_t> decrypted;
			auto decryptionResult = crypto::AesGcm256::TryDecrypt(sharedKey, encrypted, decrypted);

			// Assert:
			return decryptionResult && clearText == decrypted;
		}

		std::optional<bool> MosaicIdDerivationTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto nonce = MosaicNonce(Get<MosaicNonce::ValueType>(testCase, "mosaicNonce"));
			auto addressMainnet = ParseAddress(Get<>(testCase, "address_Public"), testCaseNumber);
			auto addressTestnet = ParseAddress(Get<>(testCase, "address_PublicTest"), testCaseNumber);

			auto expectedMosaicIdMainnet = ParseMosaicId(Get<>(testCase, "mosaicId_Public"), testCaseNumber);
			auto expectedMosaicIdTestnet = ParseMosaicId(Get<>(testCase, "mosaicId_PublicTest"), testCaseNumber);

			// Act:
			auto mosaicIdMainnet = model::GenerateMosaicId(addressMainnet, nonce);
			auto mosaicIdTestnet = model::GenerateMosaicId(addressTestnet, nonce);

			// Assert:
			return expectedMosaicIdMainnet == mosaicIdMainnet && expectedMosaicIdTestnet == mosaicIdTestnet;
		}

		std::optional<bool> Bip32DerivationTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
			auto seedStr = Get<>(testCase, "seed");
			auto seed = ParseVector(seedStr, testCaseNumber, seedStr.size() / 2);
			auto expectedRootPublicKey = ParsePublicKey(Get<>(testCase, "rootPublicKey"), testCaseNumber);

			std::vector<Key> expectedChildPublicKeys;
			for (const auto& childPair : testCase.get_child("childAccounts"))
				expectedChildPublicKeys.push_back(ParsePublicKey(Get<>(childPair.second, "publicKey"), testCaseNumber));

			// Act:
			auto node = extensions::Bip32Node::FromSeed(seed);
			auto rootPublicKey = node.publicKey();

			std::vector<Key> childPublicKeys;
			for (const auto& childPair : testCase.get_child("childAccounts")) {
				auto path = GetVector<uint32_t>(childPair.second, "path");
				childPublicKeys.push_back(node.derive(path).publicKey());
			}

			// Assert:
			return expectedRootPublicKey == rootPublicKey && expectedChildPublicKeys == childPublicKeys;
		}

		std::optional<bool> Bip39DerivationTester(const std::string&, const pt::ptree& testCase, size_t testCaseNumber) {
			// Arrange:
			auto optionalMnemonic = testCase.get_optional<std::string>("mnemonic");
			if (!optionalMnemonic.is_initialized())
				return std::optional<bool>();

			auto passphrase = Get<>(testCase, "passphrase");
			auto expectedSeed = ParseHash512(Get<>(testCase, "seed"), testCaseNumber);

			// Act:
			auto seedFromMnemonic = extensions::Bip39MnemonicToSeed(optionalMnemonic.get(), passphrase);

			// Assert:
			return expectedSeed == seedFromMnemonic;
		}

		class TestVectorsTool : public Tool {
		public:
			std::string name() const override {
				return "Test-vectors Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional& positional) override {
				optionsBuilder("vectors,v",
						OptionsValue<std::string>(m_vectorsDirectory)->required(),
						"path to test-vectors directory");

				optionsBuilder("inclusion-filter,i",
						OptionsValue<std::vector<uint16_t>>(m_inclusionFilter)->multitoken()->default_value(
								{ 0, 1, 2, 3, 4, 5, 6 },
								"all"),
						"identifiers of tests to include");

				positional.add("vectors", -1);
			}

			int run(const Options&) override {
				runTest(0, "0.test-sha3-256", "sha3", CreateHashTester(crypto::Sha3_256));
				runTest(1, "1.test-keys", "key conversion", KeyConversionTester);
				runTest(1, "1.test-address", "address conversion", AddressConversionTester);
				runTest(2, "2.test-sign", "sign", SignTester);
				runTest(2, "2.test-sign", "verify", VerifyTester);
				runTest(3, "3.test-derive", "shared key derive", DeriveTester);
				runTest(4, "4.test-cipher", "aes-gcm decryption", DecryptTester);
				runTest(5, "5.test-mosaic-id", "mosaic id derivation", MosaicIdDerivationTester);
				runTest(6, "6.test-hd-derivation", "BIP32 derivation", Bip32DerivationTester);
				runTest(6, "6.test-hd-derivation", "BIP39 derivation", Bip39DerivationTester);
				return 0;
			}

		private:
			bool shouldExecute(uint16_t testId) const {
				return m_inclusionFilter.cend() != std::find(m_inclusionFilter.cbegin(), m_inclusionFilter.cend(), testId);
			}

			pt::ptree parseJsonFile(const std::string& filename) {
				auto path = std::filesystem::path(m_vectorsDirectory) / (filename + ".json");
				pt::ptree testData;
				pt::read_json(path.generic_string(), testData);
				return testData;
			}

			void runTest(
					uint16_t testId,
					const std::string& testCaseFilename,
					const std::string& testName,
					const std::function<std::optional<bool> (const std::string&, const pt::ptree&, size_t)>& testFunc) {
				if (!shouldExecute(testId)) {
					CATAPULT_LOG(debug) << testName << " SKIPPED";
					return;
				}

				size_t testCaseNumber = 0;
				size_t numFailed = 0;
				auto testCases = parseJsonFile(testCaseFilename);
				for (const auto& testCasePair : testCases) {
					auto updateCounters = [&testCaseNumber, &numFailed](auto result) {
						if (!result.has_value())
							return;

						if (!result.value())
							++numFailed;

						++testCaseNumber;
					};

					if (!testCasePair.first.empty()) {
						for (const auto& subTestCasePair : testCasePair.second)
							updateCounters(testFunc(testCasePair.first, subTestCasePair.second, testCaseNumber));
					} else {
						updateCounters(testFunc(std::string(), testCasePair.second, testCaseNumber));
					}
				}

				if (numFailed)
					CATAPULT_LOG(warning) << testName << " test: " << numFailed << " failures out of " << testCaseNumber;
				else
					CATAPULT_LOG(info) << testName << " test: " << testCaseNumber << " successes";
			}

		private:
			std::string m_vectorsDirectory;
			std::vector<uint16_t> m_inclusionFilter;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::testvectors::TestVectorsTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
