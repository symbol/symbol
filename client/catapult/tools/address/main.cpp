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
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/Address.h"
#include "catapult/utils/HexParser.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/RandomGenerator.h"
#include "catapult/exceptions.h"
#include <iostream>
#include <string>

namespace catapult { namespace tools { namespace address {

	namespace {
		class AddressTool : public Tool {
		public:
			std::string name() const override {
				return "Address Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				optionsBuilder("generate,g",
						OptionsValue<uint32_t>(m_numRandomKeys)->default_value(3),
						"number of random keys to generate");
				optionsBuilder("public,p",
						OptionsValue<std::string>(m_publicKey),
						"show address associated with public key");
				optionsBuilder("secret,s",
						OptionsValue<std::string>(m_secretKey),
						"show address and public key associated with private key");
				optionsBuilder("network,n",
						OptionsValue<std::string>(m_networkName)->default_value("private"),
						"network, possible values: private (default), private-test, public, public-test");
				optionsBuilder("useLowEntropySource,w",
						OptionsSwitch(),
						"true if a low entropy source should be used for randomness (unsafe)");
			}

			int run(const Options& options) override {
				model::NetworkIdentifier networkIdentifier;
				if (!model::TryParseValue(m_networkName, networkIdentifier))
					CATAPULT_THROW_INVALID_ARGUMENT_1("unknown network", m_networkName);

				if (!m_publicKey.empty()) {
					output(networkIdentifier, utils::ParseByteArray<Key>(m_publicKey));
					return 0;
				}

				if (!m_secretKey.empty()) {
					output(networkIdentifier, crypto::KeyPair::FromString(m_secretKey));
					return 0;
				}

				if (options["useLowEntropySource"].as<bool>())
					generateKeys(networkIdentifier, utils::LowEntropyRandomGenerator());
				else
					generateKeys(networkIdentifier, utils::HighEntropyRandomGenerator());

				return 0;
			}

		private:
			void output(model::NetworkIdentifier networkIdentifier, const crypto::KeyPair& keyPair) {
				std::cout
						<< std::setw(Label_Width) << "private key: "
						<< crypto::Ed25519Utils::FormatPrivateKey(keyPair.privateKey()) << std::endl;
				output(networkIdentifier, keyPair.publicKey());
			}

			void output(model::NetworkIdentifier networkIdentifier, const Key& publicKey) {
				auto address = model::PublicKeyToAddress(publicKey, networkIdentifier);
				std::cout
						<< std::setw(Label_Width) << "public key: " << publicKey << std::endl
						<< std::setw(Label_Width - static_cast<int>(m_networkName.size()) - 3)
								<< "address (" << m_networkName << "): " << model::AddressToString(address) << std::endl
						<< std::setw(Label_Width) << "address decoded: " << address << std::endl;
			}

			template<typename TGenerator>
			void generateKeys(model::NetworkIdentifier networkIdentifier, TGenerator&& generator) {
				std::cout << "--- generating " << m_numRandomKeys << " keys ---" << std::endl;

				for (auto i = 0u; i < m_numRandomKeys; ++i) {
					output(networkIdentifier, crypto::KeyPair::FromPrivate(crypto::PrivateKey::Generate([&generator]() {
						return static_cast<uint8_t>(generator());
					})));
					std::cout << std::endl;
				}
			}

		private:
			uint32_t m_numRandomKeys;
			std::string m_publicKey;
			std::string m_secretKey;
			std::string m_networkName;

		private:
			static constexpr int Label_Width = 24;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::address::AddressTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
