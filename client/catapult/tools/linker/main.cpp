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
#include "tools/ToolConfigurationUtils.h"
#include "catapult/builders/VotingKeyLinkBuilder.h"
#include "catapult/builders/VrfKeyLinkBuilder.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/TransactionExtensions.h"
#include "catapult/io/RawFile.h"
#include "catapult/utils/HexParser.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace tools { namespace linker {

	namespace {
		void SaveTransaction(const model::Transaction& transaction, const std::string& filePath) {
			io::RawFile dataFile(filePath, io::OpenMode::Read_Write);
			dataFile.write({ reinterpret_cast<const uint8_t*>(&transaction), transaction.Size });
		}

		class LinkerTool : public Tool {
		public:
			std::string name() const override {
				return "Linker Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				optionsBuilder("resources,r",
						OptionsValue<std::string>()->default_value(".."),
						"the path to the resources directory");

				optionsBuilder("type",
						OptionsValue<std::string>()->default_value("vrf"),
						"link transaction type: voting, vrf");
				optionsBuilder("secret,s",
						OptionsValue<std::string>(),
						"secret key to sign the transaction");
				optionsBuilder("linkedPublicKey",
						OptionsValue<std::string>(),
						"linked public key (32 bytes for vrf, 48 bytes for voting)");
				optionsBuilder("output",
						OptionsValue<std::string>(),
						"output filename");
			}

			int run(const Options& options) override {
				auto config = LoadConfiguration(options["resources"].as<std::string>());
				validateOptions(options);

				// 1. create transaction
				auto signer = crypto::KeyPair::FromString(options["secret"].as<std::string>());
				auto linkedPublicKey = options["linkedPublicKey"].as<std::string>();
				auto networkIdentifier = config.BlockChain.Network.Identifier;
				auto pTransaction = options["type"].as<std::string>() == "voting"
						? createVotingKeyLinkTransaction(networkIdentifier, signer.publicKey(), linkedPublicKey)
						: createVrfKeyLinkTransaction(networkIdentifier, signer.publicKey(), linkedPublicKey);

				// 2. sign it
				pTransaction->Deadline = Timestamp(1);
				auto transactionExtensions = extensions::TransactionExtensions(config.BlockChain.Network.GenerationHashSeed);
				transactionExtensions.sign(signer, *pTransaction);
				auto transactionHash = transactionExtensions.hash(*pTransaction);

				// 3. save it
				SaveTransaction(*pTransaction, options["output"].as<std::string>());
				CATAPULT_LOG(info) << "saved transaction from " << signer.publicKey() << " with hash: " << transactionHash;
				return 0;
			}

		private:
			void validateOptions(const Options& options) {
				if (options["type"].as<std::string>() != "voting" && options["type"].as<std::string>() != "vrf")
					CATAPULT_THROW_INVALID_ARGUMENT("invalid --type argument");

				if (options["secret"].empty())
					CATAPULT_THROW_INVALID_ARGUMENT("missing secret key");

				if (options["output"].empty())
					CATAPULT_THROW_INVALID_ARGUMENT("missing output name path");

				if (boost::filesystem::exists(options["output"].as<std::string>()))
					CATAPULT_THROW_INVALID_ARGUMENT("output file already exists");
			}

			std::shared_ptr<model::Transaction> createVotingKeyLinkTransaction(
					model::NetworkIdentifier networkIdentifier,
					const Key& publicKey,
					const std::string& linkedPublicKey) {
				builders::VotingKeyLinkBuilder builder(networkIdentifier, publicKey);
				builder.setLinkedPublicKey(utils::ParseByteArray<VotingKey>(linkedPublicKey));
				builder.setLinkAction(model::LinkAction::Link);
				return builder.build();
			}

			std::shared_ptr<model::Transaction> createVrfKeyLinkTransaction(
					model::NetworkIdentifier networkIdentifier,
					const Key& publicKey,
					const std::string& linkedPublicKey) {
				builders::VrfKeyLinkBuilder builder(networkIdentifier, publicKey);
				builder.setLinkedPublicKey(utils::ParseByteArray<Key>(linkedPublicKey));
				builder.setLinkAction(model::LinkAction::Link);
				return builder.build();
			}
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::linker::LinkerTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
