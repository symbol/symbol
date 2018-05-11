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

#include "catapult/crypto/KeyGenerator.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/crypto/PrivateKey.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/Address.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/HexParser.h"
#include "tests/TestHarness.h"
#include <boost/algorithm/string/split.hpp>
#include <cctype>
#include <fstream>

namespace catapult {

#define TEST_CLASS TestVectorsTests

	namespace {
		template<typename TLineParser, typename TAction>
		void RunTest(const std::string& sourceFilename, TLineParser lineParser, TAction action) {
#ifdef SIGNATURE_SCHEME_NIS1
			constexpr auto Source_Directory = "../tests/int/stress/resources/nis1/";
#else
			constexpr auto Source_Directory = "../tests/int/stress/resources/catapult/";
#endif

			// Arrange:
			auto i = 0u;
			std::ifstream input(Source_Directory + sourceFilename);
			for (std::string line; getline(input, line);) {
				if (line.empty() || '#' == line[0])
					continue;

				// - strip all spaces from the string
				line.erase(std::remove_if(line.begin(), line.end(), [](auto ch) { return std::isspace(ch); }), line.end());

				// - split the line on colons
				std::vector<std::string> parts;
				boost::algorithm::split(parts, line, [](auto ch) { return ':' == ch; });

				// - parse the line
				auto result = lineParser(parts);
				if (!result.second) {
					FAIL() << "malformed line: " << line;
					continue;
				}

				// Act + Assert:
				action(result.first);
				++i;
			}

			CATAPULT_LOG(debug) << "executed " << i << " test cases";
		}
	}

	// region test-keys

	namespace {
		struct KeyTestData {
			crypto::PrivateKey PrivateKey;
			Key PublicKey;
			catapult::Address Address;
		};

		std::pair<KeyTestData, bool> ParseKeyTestData(const std::vector<std::string>& parts) {
			// indexes into parsed line
			enum : size_t { Placeholder, Private_Key, Private_Key_Nis_Format, Public_Key, Address, Max };

			if (Max != parts.size())
				return std::make_pair(KeyTestData(), false);

			KeyTestData data;
			data.PrivateKey = crypto::PrivateKey::FromString(parts[Private_Key]);
			data.PublicKey = crypto::ParseKey(parts[Public_Key]);
			data.Address = model::StringToAddress(parts[Address]);
			return std::make_pair(std::move(data), true);
		}
	}

	TEST(TEST_CLASS, KeysTestVectors) {
		// Arrange:
		RunTest("1.test-keys.dat", ParseKeyTestData, [](const auto& testData) {
			// Act:
			Key publicKey;
			crypto::ExtractPublicKeyFromPrivateKey(testData.PrivateKey, publicKey);
			auto address = model::PublicKeyToAddress(testData.PublicKey, model::NetworkIdentifier::Public);

			// Assert:
			EXPECT_EQ(testData.PublicKey, publicKey) << "address " << model::AddressToString(testData.Address);
			EXPECT_EQ(testData.Address, address) << "address " << model::AddressToString(testData.Address);
		});
	}

	// endregion

	// region test-sign

	namespace {
		struct SignTestData {
		public:
			explicit SignTestData(crypto::PrivateKey&& privateKey) : KeyPair(crypto::KeyPair::FromPrivate(std::move(privateKey)))
			{}

		public:
			crypto::KeyPair KeyPair;
			catapult::Signature Signature;
			std::vector<uint8_t> Data;
		};

		std::pair<SignTestData, bool> ParseSignTestData(const std::vector<std::string>& parts) {
			// indexes into parsed line
			enum : size_t { Placeholder, Private_Key, Public_Key, Signature, Data_Length, Data, Max };

			if (Max != parts.size())
				return std::make_pair(SignTestData(crypto::PrivateKey()), false);

			SignTestData data(crypto::PrivateKey::FromString(parts[Private_Key]));
			utils::ParseHexStringIntoContainer(parts[Signature].c_str(), parts[Signature].size(), data.Signature);

			auto numDataBytes = static_cast<size_t>(std::atoi(parts[Data_Length].c_str()));
			data.Data.resize(numDataBytes);
			utils::ParseHexStringIntoContainer(parts[Data].c_str(), parts[Data].size(), data.Data);
			return std::make_pair(std::move(data), true);
		}
	}

	TEST(TEST_CLASS, SignTestVectors) {
		// Arrange:
		RunTest("2.test-sign.dat", ParseSignTestData, [](const auto& testData) {
			// Act:
			Signature signature;
			crypto::Sign(testData.KeyPair, testData.Data, signature);

			// Assert:
			EXPECT_EQ(testData.Signature, signature) << "data " << utils::HexFormat(testData.Data);
		});
	}

	// endregion
}
