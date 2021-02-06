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
#include "tools/ToolConfigurationUtils.h"
#include "catapult/builders/AccountKeyLinkBuilder.h"
#include "catapult/builders/VotingKeyLinkBuilder.h"
#include "catapult/builders/VrfKeyLinkBuilder.h"
#include "catapult/extensions/TransactionExtensions.h"
#include "catapult/io/RawFile.h"
#include "catapult/utils/HexParser.h"
#include <filesystem>

namespace catapult { namespace tools { namespace linker {

	namespace {
		// region key type

		enum class KeyType { Voting, Vrf, Remote };

		KeyType ParseKeyType(const std::string& str) {
			static const std::array<std::pair<const char*, KeyType>, 3> String_To_KeyType_Pairs{{
				{ "voting", KeyType::Voting },
				{ "vrf", KeyType::Vrf },
				{ "remote", KeyType::Remote }
			}};

			KeyType keyType;
			if (!utils::TryParseEnumValue(String_To_KeyType_Pairs, str, keyType)) {
				std::ostringstream out;
				out << "'" << str << "' is not a valid link key type";
				CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
			}

			return keyType;
		}

		// endregion

		// region builder wrappers

		class KeyLinkTransactionFactory {
		public:
			KeyLinkTransactionFactory(model::NetworkIdentifier networkIdentifier, const Key& signerPublicKey)
					: m_networkIdentifier(networkIdentifier)
					, m_signerPublicKey(signerPublicKey)
			{}

		public:
			auto create(KeyType keyType, const std::string& linkedPublicKey, FinalizationEpoch startEpoch, FinalizationEpoch endEpoch) {
				switch (keyType) {
					case KeyType::Voting:
						return createVotingKeyLinkTransaction(linkedPublicKey, startEpoch, endEpoch);
					case KeyType::Vrf:
						return createVrfKeyLinkTransaction(linkedPublicKey);
					case KeyType::Remote:
						return createRemoteKeyLinkTransaction(linkedPublicKey);
				}

				return std::shared_ptr<model::Transaction>();
			}

		private:
			std::shared_ptr<model::Transaction> createVotingKeyLinkTransaction(
					const std::string& linkedPublicKey,
					FinalizationEpoch startEpoch,
					FinalizationEpoch endEpoch) {
				builders::VotingKeyLinkBuilder builder(m_networkIdentifier, m_signerPublicKey);
				builder.setLinkedPublicKey(utils::ParseByteArray<VotingKey>(linkedPublicKey));
				builder.setLinkAction(model::LinkAction::Link);
				builder.setStartEpoch(startEpoch);
				builder.setEndEpoch(endEpoch);
				return builder.build();
			}

			std::shared_ptr<model::Transaction> createVrfKeyLinkTransaction(const std::string& linkedPublicKey) {
				builders::VrfKeyLinkBuilder builder(m_networkIdentifier, m_signerPublicKey);
				builder.setLinkedPublicKey(utils::ParseByteArray<Key>(linkedPublicKey));
				builder.setLinkAction(model::LinkAction::Link);
				return builder.build();
			}

			std::shared_ptr<model::Transaction> createRemoteKeyLinkTransaction(const std::string& linkedPublicKey) {
				builders::AccountKeyLinkBuilder builder(m_networkIdentifier, m_signerPublicKey);
				builder.setLinkedPublicKey(utils::ParseByteArray<Key>(linkedPublicKey));
				builder.setLinkAction(model::LinkAction::Link);
				return builder.build();
			}

		private:
			model::NetworkIdentifier m_networkIdentifier;
			Key m_signerPublicKey;
		};

		// endregion

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
				AddResourcesOption(optionsBuilder);

				optionsBuilder("type",
						OptionsValue<std::string>()->default_value("vrf"),
						"link transaction type: voting, vrf, remote");
				optionsBuilder("secret,s",
						OptionsValue<std::string>(),
						"secret key to sign the transaction");
				optionsBuilder("linkedPublicKey",
						OptionsValue<std::string>(),
						"linked public key");
				optionsBuilder("startEpoch,b",
						OptionsValue<uint32_t>()->default_value(1),
						"voting key start epoch");
				optionsBuilder("endEpoch,e",
						OptionsValue<uint32_t>()->default_value(26280),
						"voting key end epoch");
				optionsBuilder("output",
						OptionsValue<std::string>(),
						"output filename");
			}

			int run(const Options& options) override {
				auto config = LoadConfiguration(GetResourcesOptionValue(options));
				validateOptions(options);

				// 1. create transaction
				auto networkIdentifier = config.BlockChain.Network.Identifier;
				auto signer = crypto::KeyPair::FromString(options["secret"].as<std::string>());
				auto linkedPublicKey = options["linkedPublicKey"].as<std::string>();
				auto keyType = ParseKeyType(options["type"].as<std::string>());
				auto startEpoch = FinalizationEpoch(options["startEpoch"].as<uint32_t>());
				auto endEpoch = FinalizationEpoch(options["endEpoch"].as<uint32_t>());

				KeyLinkTransactionFactory factory(networkIdentifier, signer.publicKey());
				auto pTransaction = factory.create(keyType, linkedPublicKey, startEpoch, endEpoch);

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
				if (options["secret"].empty())
					CATAPULT_THROW_INVALID_ARGUMENT("missing secret key");

				if (options["output"].empty())
					CATAPULT_THROW_INVALID_ARGUMENT("missing output name path");

				if (std::filesystem::exists(options["output"].as<std::string>()))
					CATAPULT_THROW_INVALID_ARGUMENT("output file already exists");
			}
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::linker::LinkerTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
