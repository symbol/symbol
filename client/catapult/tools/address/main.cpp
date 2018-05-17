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
#include "tools/Random.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/model/Address.h"
#include "catapult/utils/Logging.h"
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
						OptionsValue<uint32_t>(m_numRandomKeys)->default_value(10),
						"number of random keys to generate");
				optionsBuilder("secret,s",
						OptionsValue<std::string>(m_secretKey),
						"private key");
				optionsBuilder("network,n",
						OptionsValue<std::string>(m_networkName)->default_value("public"),
						"network, possible values: mijin, mijin-test, public (default), public-test");
			}

			int run(const Options&) override {
				model::NetworkIdentifier networkId;
				if (!model::TryParseValue(m_networkName, networkId))
					CATAPULT_THROW_INVALID_ARGUMENT_1("unknown network", m_networkName);

				if (!m_secretKey.empty()) {
					output(networkId, crypto::KeyPair::FromString(m_secretKey));
					return 0;
				}

				std::cout << "--- generating " << m_numRandomKeys << " keys ---" << std::endl;
				for (auto i = 0u; i < m_numRandomKeys; ++i) {
					output(networkId, crypto::KeyPair::FromPrivate(crypto::PrivateKey::Generate(RandomByte)));
					std::cout << std::endl;
				}

				return 0;
			}

		private:
			void output(model::NetworkIdentifier networkId, const crypto::KeyPair& keyPair) {
				static constexpr int Label_Width = 24;

				std::cout << std::setw(Label_Width) << "private key: " << crypto::FormatKey(keyPair.privateKey()) << std::endl;
				std::cout << std::setw(Label_Width) << "public key: " << crypto::FormatKey(keyPair.publicKey()) << std::endl;
				std::cout << std::setw(Label_Width - static_cast<int>(m_networkName.size()) - 3)
						<< "address (" << m_networkName << "): "
						<< model::AddressToString(model::PublicKeyToAddress(keyPair.publicKey(), networkId)) << std::endl;
			}

		private:
			uint32_t m_numRandomKeys;
			std::string m_secretKey;
			std::string m_networkName;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::address::AddressTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
