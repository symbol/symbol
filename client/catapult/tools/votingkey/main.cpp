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
#include "catapult/crypto/SecureRandomGenerator.h"
#include "catapult/crypto_voting/BmPrivateKeyTree.h"
#include "catapult/io/FileStream.h"
#include "catapult/model/StepIdentifier.h"
#include "catapult/exceptions.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

namespace catapult { namespace tools { namespace votingkey {

	namespace {
		class VotingKeyTool : public Tool {
		private:
			using BmPublicKey = VotingKey;
			using BmKeyPair = crypto::VotingKeyPair;

		public:
			std::string name() const override {
				return "Voting Key Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				optionsBuilder("output,o",
						OptionsValue<std::string>(m_filename)->default_value("voting_private_key_tree.dat"),
						"voting private key tree file");
				optionsBuilder("dilution,d",
						OptionsValue<uint16_t>(m_dilution)->default_value(128),
						"voting key dilution (network setting)");
				optionsBuilder("startEpoch,b",
						OptionsValue<uint32_t>(m_startEpoch)->default_value(1),
						"voting key start epoch");
				optionsBuilder("endEpoch,e",
						OptionsValue<uint32_t>(m_endEpoch)->default_value(26280),
						"voting key end epoch");
				optionsBuilder("secret,s",
						OptionsValue<std::string>(m_secretKey),
						"root secret key (testnet only, don't use in production)");
			}

			int run(const Options&) override {
				crypto::SecureRandomGenerator generator;
				auto keyPair = BmKeyPair::FromPrivate(BmKeyPair::PrivateKey::Generate([&generator] {
					return static_cast<uint8_t>(generator());
				}));

				if (!m_secretKey.empty())
					keyPair = BmKeyPair::FromString(m_secretKey);

				auto savedPublicKey = generateTree(std::move(keyPair));
				auto loadedPublicKey = verifyFile();

				std::cout << " saved voting public key: " << savedPublicKey << std::endl;
				std::cout << "loaded voting public key: " << loadedPublicKey << std::endl;
				return savedPublicKey != loadedPublicKey;
			}

		private:
			crypto::BmKeyIdentifier toKeyIdentifier(FinalizationEpoch epoch, model::FinalizationStage stage) const {
				return model::StepIdentifierToBmKeyIdentifier({ epoch, FinalizationPoint(), stage }, m_dilution);
			}

			BmPublicKey generateTree(crypto::VotingKeyPair&& keyPair) {
				if (boost::filesystem::exists(m_filename))
					CATAPULT_THROW_RUNTIME_ERROR("voting private key tree file already exits");

				io::FileStream stream(io::RawFile(m_filename, io::OpenMode::Read_Write));
				crypto::BmOptions options;
				options.Dilution = m_dilution;
				options.StartKeyIdentifier = toKeyIdentifier(FinalizationEpoch(m_startEpoch), model::FinalizationStage::Prevote);
				options.EndKeyIdentifier = toKeyIdentifier(FinalizationEpoch(m_endEpoch), model::FinalizationStage::Precommit);

				auto numBatches = (options.EndKeyIdentifier.BatchId - options.StartKeyIdentifier.BatchId + 1);
				std::cout << "generating " << numBatches << " batch keys, this might take a while" << std::endl;

				auto tree = crypto::BmPrivateKeyTree::Create(std::move(keyPair), stream, options);
				std::cout << m_filename << " generated" << std::endl;
				return tree.rootPublicKey();
			}

			BmPublicKey verifyFile() {
				std::cout << "verifying generated file" << std::endl;
				io::FileStream stream(io::RawFile(m_filename, io::OpenMode::Read_Only));
				auto tree = crypto::BmPrivateKeyTree::FromStream(stream);
				return tree.rootPublicKey();
			}

		private:
			std::string m_filename;
			uint16_t m_dilution;
			uint32_t m_startEpoch;
			uint32_t m_endEpoch;
			std::string m_secretKey;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::votingkey::VotingKeyTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
