#include "NemesisConfiguration.h"
#include "TransactionRegistryFactory.h"
#include "tools/ToolMain.h"
#include "catapult/builders/MosaicDefinitionBuilder.h"
#include "catapult/builders/MosaicSupplyChangeBuilder.h"
#include "catapult/builders/RegisterNamespaceBuilder.h"
#include "catapult/builders/TransferBuilder.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/extensions/BlockExtensions.h"
#include "catapult/extensions/IdGenerator.h"
#include "catapult/extensions/TransactionExtensions.h"
#include "catapult/io/FileBasedStorage.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/MosaicSupplyChangeTransaction.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/HexParser.h"
#include <boost/filesystem.hpp>
#include <string>

namespace catapult { namespace tools { namespace nemgen {

	namespace {
		// region config

		template<typename TIdentifier>
		void OutputName(const std::string& name, TIdentifier id) {
			CATAPULT_LOG(debug) << " - " << name << " (" << utils::HexFormat(id) << ")";
		}

		NemesisConfiguration LoadConfiguration(const boost::filesystem::path& filePath) {
			if (!boost::filesystem::exists(filePath)) {
				auto message = "aborting load due to missing configuration file";
				CATAPULT_LOG(fatal) << message << ": " << filePath;
				CATAPULT_THROW_EXCEPTION(catapult_runtime_error(message));
			}

			CATAPULT_LOG(info) << "loading nemesis configuration from " << filePath;
			return NemesisConfiguration::LoadFromBag(utils::ConfigurationBag::FromPath(filePath.generic_string()));
		}

		void LogNamespaces(const NemesisConfiguration& config) {
			CATAPULT_LOG(debug) << "Namespace Summary";
			for (const auto& pair : config.RootNamespaces) {
				const auto& root = pair.second;
				const auto& name = config.NamespaceNames.at(root.id());
				OutputName(name, root.id());
				CATAPULT_LOG(debug) << " - Owner: " << utils::HexFormat(root.owner());
				CATAPULT_LOG(debug) << " - Start Height: " << root.lifetime().Start;
				CATAPULT_LOG(debug) << " - End Height: " << root.lifetime().End;
				if (!root.empty()) {
					CATAPULT_LOG(debug) << " - Children:";
					for (const auto & childPair : root.children()) {
						const auto& childName = config.NamespaceNames.at(childPair.first);
						CATAPULT_LOG(debug) << " - - " << childName << " (" << utils::HexFormat(childPair.first) << ")";
					}
				}

				CATAPULT_LOG(debug);
			}
		}

		bool LogMosaicDefinitions(const NemesisConfiguration& config) {
			CATAPULT_LOG(debug) << "Mosaic Summary";
			std::unordered_set<MosaicId, utils::BaseValueHasher<MosaicId>> mosaicIds;
			for (const auto& pair : config.MosaicEntries) {
				const std::string& name = pair.first;
				auto id = pair.second.mosaicId();
				const auto& mosaicEntry = pair.second;
				const auto& definition = mosaicEntry.definition();
				const auto& properties = definition.properties();
				OutputName(name, id);
				CATAPULT_LOG(debug) << " - Namespace Id: " << utils::HexFormat(mosaicEntry.namespaceId());
				CATAPULT_LOG(debug) << " - Owner: " << utils::HexFormat(definition.owner());
				CATAPULT_LOG(debug) << " - Supply: " << mosaicEntry.supply();
				CATAPULT_LOG(debug) << " - Divisibility: " << static_cast<uint32_t>(properties.divisibility());
				CATAPULT_LOG(debug) << " - Duration: " << properties.duration() << " blocks (0 = eternal)";
				CATAPULT_LOG(debug) << " - IsTransferable: " << properties.is(model::MosaicFlags::Transferable);
				CATAPULT_LOG(debug) << " - IsSupplyMutable: " << properties.is(model::MosaicFlags::Supply_Mutable);
				CATAPULT_LOG(debug) << " - IsLevyMutable: " << properties.is(model::MosaicFlags::Levy_Mutable);
				CATAPULT_LOG(debug);

				if (!mosaicIds.insert(id).second) {
					CATAPULT_LOG(warning) << "mosaic " << name << " does not have a unique id";
					return false;
				}
			}

			if (config.MosaicEntries.cend() == config.MosaicEntries.find("nem:xem")) {
				CATAPULT_LOG(warning) << "nem:xem must be included in nemesis block";
				return false;
			}

			return true;
		}

		bool LogMosaicDistribution(const NemesisConfiguration& config) {
			CATAPULT_LOG(debug) << "Nemesis Seed Amounts";
			for (const auto& addressMosaicSeedsPair : config.NemesisAddressToMosaicSeeds) {
				const auto& address = addressMosaicSeedsPair.first;
				CATAPULT_LOG(debug) << " - " << address;
				if (!model::IsValidEncodedAddress(address, config.NetworkIdentifier)) {
					CATAPULT_LOG(warning) << "address " << address << " is invalid";
					return false;
				}

				for (const auto& seed : addressMosaicSeedsPair.second)
					CATAPULT_LOG(debug) << " - - " << seed.Name << ": " << seed.Amount;
			}

			return true;
		}

		bool LogAndValidateConfiguration(const NemesisConfiguration& config) {
			CATAPULT_LOG(debug) << "--- Nemesis Configuration ---";
			CATAPULT_LOG(debug) << "Network            : " << config.NetworkIdentifier;
			CATAPULT_LOG(debug) << "Nemesis Gen Hash   : " << config.NemesisGenerationHash;
			CATAPULT_LOG(debug) << "Nemesis Private Key: " << config.NemesisSignerPrivateKey;
			CATAPULT_LOG(debug) << "Cpp File           : " << config.CppFile;
			CATAPULT_LOG(debug) << "Bin Directory      : " << config.BinDirectory;

			// - namespaces
			LogNamespaces(config);

			// - mosaic definitions and distribution
			return LogMosaicDefinitions(config) && LogMosaicDistribution(config);
		}

		// endregion

		// region block generation

		auto ExtractMosaicName(const std::string& fqn) {
			auto pos = fqn.find_last_of(':');
			return fqn.substr(pos + 1);
		}

		class NemesisTransactions {
		public:
			explicit NemesisTransactions(model::NetworkIdentifier networkIdentifier, const crypto::KeyPair& signer)
					: m_networkIdentifier(networkIdentifier)
					, m_signer(signer)
			{}

		public:
			void addRegisterNamespace(const std::string& namespaceName, ArtifactDuration duration) {
				builders::RegisterNamespaceBuilder builder(m_networkIdentifier, m_signer.publicKey(), namespaceName);
				builder.setDuration(duration);
				signAndAdd(builder.build());
			}

			void addRegisterNamespace(const std::string& namespaceName, NamespaceId parentId) {
				builders::RegisterNamespaceBuilder builder(m_networkIdentifier, m_signer.publicKey(), namespaceName);
				builder.setParentId(parentId);
				signAndAdd(builder.build());
			}

			void addMosaicDefinition(
					NamespaceId parentId,
					const std::string& mosaicName,
					const model::MosaicProperties& properties) {
				builders::MosaicDefinitionBuilder builder(m_networkIdentifier, m_signer.publicKey(), parentId, mosaicName);
				builder.setDivisibility(properties.divisibility());
				builder.setDuration(properties.duration());
				if (properties.is(model::MosaicFlags::Transferable))
					builder.setTransferable();

				if (properties.is(model::MosaicFlags::Supply_Mutable))
					builder.setSupplyMutable();

				if (properties.is(model::MosaicFlags::Levy_Mutable))
					builder.setLevyMutable();

				signAndAdd(builder.build());
			}

			void addMosaicSupplyChange(MosaicId mosaicId, Amount delta) {
				builders::MosaicSupplyChangeBuilder builder(m_networkIdentifier, m_signer.publicKey(), mosaicId);
				builder.setDelta(delta);
				auto pTransaction = builder.build();
				signAndAdd(std::move(pTransaction));
			}

			void addTransfer(const Address& recipientAddress, const std::vector<MosaicSeed>& seeds) {
				builders::TransferBuilder builder(m_networkIdentifier, m_signer.publicKey(), recipientAddress);
				for (const auto& seed : seeds)
					builder.addMosaic(seed.Name, seed.Amount);

				signAndAdd(builder.build());
			}

		public:
			const model::Transactions& transactions() const {
				return m_transactions;
			}

		private:
			void signAndAdd(std::unique_ptr<model::Transaction>&& pTransaction) {
				pTransaction->Deadline = Timestamp(1);
				extensions::SignTransaction(m_signer, *pTransaction);
				m_transactions.push_back(std::move(pTransaction));
			}

		private:
			model::NetworkIdentifier m_networkIdentifier;
			const crypto::KeyPair& m_signer;
			model::Transactions m_transactions;
		};

		std::unique_ptr<model::Block> CreateNemesisBlock(const NemesisConfiguration& config) {
			auto signer = crypto::KeyPair::FromString(config.NemesisSignerPrivateKey);

			NemesisTransactions transactions(config.NetworkIdentifier, signer);

			// - namespace creation
			for (const auto& rootPair : config.RootNamespaces) {
				// - root
				const auto& root = rootPair.second;
				const auto& rootName = config.NamespaceNames.at(root.id());
				auto duration = std::numeric_limits<ArtifactDuration::ValueType>::max() == root.lifetime().End.unwrap()
						? Eternal_Artifact_Duration
						: ArtifactDuration((root.lifetime().End - root.lifetime().Start).unwrap());
				transactions.addRegisterNamespace(rootName, duration);

				// - children
				std::map<size_t, std::vector<state::Namespace::Path>> paths;
				for (const auto& childPair : root.children())
					paths[childPair.second.size()].push_back(childPair.second);

				for (const auto& pair : paths) {
					for (const auto& path : pair.second) {
						const auto& child = state::Namespace(path);
						const auto& childName = config.NamespaceNames.at(child.id());
						transactions.addRegisterNamespace(childName, child.parentId());
					}
				}
			}

			// - mosaic creation
			for (const auto& mosaicPair : config.MosaicEntries) {
				const auto& mosaicName = mosaicPair.first;
				const auto& mosaicEntry = mosaicPair.second;

				// - definition
				transactions.addMosaicDefinition(
						mosaicEntry.namespaceId(),
						ExtractMosaicName(mosaicName),
						mosaicEntry.definition().properties());

				// - supply
				transactions.addMosaicSupplyChange(mosaicEntry.mosaicId(), mosaicEntry.supply());
			}

			// - mosaic distribution
			for (const auto& addressMosaicSeedsPair : config.NemesisAddressToMosaicSeeds)
				transactions.addTransfer(model::StringToAddress(addressMosaicSeedsPair.first), addressMosaicSeedsPair.second);

			model::PreviousBlockContext context;
			auto pBlock = model::CreateBlock(context, config.NetworkIdentifier, signer.publicKey(), transactions.transactions());
			pBlock->Type = model::EntityType::Nemesis_Block;
			extensions::SignFullBlock(signer, *pBlock);
			return pBlock;
		}

		// endregion

		// region block saving

		class TempZeroIndexFile {
		public:
			explicit TempZeroIndexFile(const boost::filesystem::path& binDirectory) : m_indexFilePath(binDirectory / "index.dat") {
				io::RawFile indexFile(m_indexFilePath.generic_string(), io::OpenMode::Read_Write);
				uint8_t zero[8] = { 0 };
				indexFile.write(RawBuffer(zero, sizeof(zero)));
			}

			~TempZeroIndexFile() {
				boost::filesystem::remove(m_indexFilePath);
			}

		private:
			boost::filesystem::path m_indexFilePath;
		};

		void UpdateMemoryBasedStorageData(const model::Block& block, const std::string& cppFile) {
			io::RawFile dataFile(cppFile, io::OpenMode::Read_Write);
			auto header = "#include \"MemoryBasedStorage.h\"\n\n"
				"namespace catapult { namespace mocks {\n\n"
				"\tconst unsigned char MemoryBasedStorage_NemesisBlockData[] = {\n";
			dataFile.write(RawBuffer(reinterpret_cast<const uint8_t*>(header), strlen(header)));

			auto pCurrent = reinterpret_cast<const uint8_t*>(&block);
			auto pEnd = pCurrent + block.Size;
			std::stringstream buffer;
			while (pCurrent != pEnd) {
				buffer << "\t\t";

				auto lineEnd = std::min(pCurrent + 16, pEnd);
				for (; pCurrent != lineEnd; ++pCurrent)
					buffer << "0x" << utils::HexFormat(*pCurrent) << ((pCurrent + 1 == lineEnd) ? "," : ", ");

				buffer << "\n";
			}

			dataFile.write(RawBuffer(reinterpret_cast<const uint8_t*>(buffer.str().c_str()), buffer.str().size()));

			auto footer = "\t};\n}}\n";
			dataFile.write(RawBuffer(reinterpret_cast<const uint8_t*>(footer), strlen(footer)));
		}

		Hash256 ParseHash(const std::string& hashString) {
			Hash256 hash;
			utils::ParseHexStringIntoContainer(hashString.c_str(), hashString.size(), hash);
			return hash;
		}

		void UpdateFileBasedStorageData(const model::Block& block, const Hash256& generationHash, const std::string& binDirectory) {
			auto registry = CreateTransactionRegistry();
			io::FileBasedStorage storage(binDirectory);
			auto blockElement = extensions::ConvertBlockToBlockElement(block, generationHash, registry);
			CATAPULT_LOG(info) << "nemesis block hash: " << utils::HexFormat(blockElement.EntityHash);
			storage.saveBlock(blockElement);
		}

		void SaveBlock(const model::Block& block, const NemesisConfiguration& config) {
			// 1. temporarily zero the index file
			TempZeroIndexFile zeroIndexFile(config.BinDirectory);

			// 2. update the file based storage data
			CATAPULT_LOG(info) << "creating binary storage seed in " << config.BinDirectory;
			UpdateFileBasedStorageData(block, ParseHash(config.NemesisGenerationHash), config.BinDirectory);

			// 3. update the memory based storage data
			if (!config.CppFile.empty()) {
				CATAPULT_LOG(info) << "creating cpp file " << config.CppFile;
				UpdateMemoryBasedStorageData(block, config.CppFile);
			}
		}

		// endregion

		class NemGenTool : public Tool {
		public:
			std::string name() const override {
				return "Nemesis Block Generator Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional& positional) override {
				optionsBuilder("nemesisProperties,r",
						OptionsValue<std::string>(m_nemesisPropertiesFilePath),
						"the path to the nemesis properties file");
				positional.add("nemesisProperties", -1);
			}

			int run(const Options&) override {
				// 1. load config
				auto config = LoadConfiguration(m_nemesisPropertiesFilePath);
				if (!LogAndValidateConfiguration(config))
					return -1;

				// 2. create and save the nemesis block
				auto pBlock = CreateNemesisBlock(config);
				SaveBlock(*pBlock, config);
				return 0;
			}

		private:
			std::string m_nemesisPropertiesFilePath;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::nemgen::NemGenTool nemGenTool;
	return catapult::tools::ToolMain(argc, argv, nemGenTool);
}
