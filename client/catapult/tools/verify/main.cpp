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
#include "tools/plugins/PluginLoader.h"
#include "tools/ToolConfigurationUtils.h"
#include "tools/ToolThreadUtils.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/extensions/BlockExtensions.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "catapult/io/BufferInputStreamAdapter.h"
#include "catapult/io/FileBlockStorage.h"
#include "catapult/io/FileProofStorage.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/model/EntityHasher.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/thread/ParallelFor.h"
#include <thread>

namespace catapult { namespace tools { namespace verify {

	namespace {
		// region ErrorType / ErrorAggregator

		enum class ErrorType {
			Block_Link,
			Importance_Link,
			Block_Type,
			Block_Signature,

			Block_Entity_Hash,
			Block_Transactions_Hash,
			Block_Receipts_Hash,

			Proof_Hash
		};

		class ErrorAggregator {
		public:
			ErrorAggregator() : m_maxHeightDigits(0)
			{}

		public:
			bool empty() const {
				return m_errorMap.empty();
			}

			std::string str() const {
				std::ostringstream out;
				for (const auto& pair : m_errorMap) {
					out << ToString(pair.first) << std::endl;
					for (const auto& description : pair.second)
						out << " - " << description << std::endl;
				}

				return out.str();
			}

		public:
			void setHeight(Height height) {
				m_height = height;
			}

			void setMaxHeight(Height height) {
				m_maxHeightDigits = 0;

				auto rawHeight = height.unwrap();
				while (rawHeight > 0) {
					++m_maxHeightDigits;
					rawHeight /= 10;
				}
			}

			void add(ErrorType errorType) {
				std::ostringstream out;
				outputHeight(out, m_height);
				out << " failed";
				m_errorMap[errorType].push_back(out.str());
			}

			template<typename T>
			void requireEqual(ErrorType errorType, const T& computed, const T& actual) {
				if (computed == actual)
					return;

				std::ostringstream out;
				outputHeight(out, m_height);
				out << " computed " << computed << " but was " << actual;
				m_errorMap[errorType].push_back(out.str());
			}

		private:
			void outputHeight(std::ostream& out, Height height) const {
				out << "[H " << std::setw(m_maxHeightDigits) << height.unwrap() << "]";
			}

		public:
			static ErrorAggregator Aggregate(std::vector<ErrorAggregator>&& errorGroups) {
				ErrorAggregator allErrors;
				for (auto& errors : errorGroups) {
					for (auto& pair : errors.m_errorMap) {
						auto& destination = allErrors.m_errorMap[pair.first];
						destination.insert(
								destination.end(),
								std::make_move_iterator(pair.second.begin()),
								std::make_move_iterator(pair.second.end()));
					}
				}

				return allErrors;
			}

		private:
			static std::string ToString(ErrorType errorType) {
				switch (errorType) {
				case ErrorType::Block_Link:
					return "detected improper block link";
				case ErrorType::Importance_Link:
					return "detected improper importance link";
				case ErrorType::Block_Type:
					return "detected unexpected block type";
				case ErrorType::Block_Signature:
					return "detected signature failure";

				case ErrorType::Block_Entity_Hash:
					return "detected block entity hash mismatch";
				case ErrorType::Block_Transactions_Hash:
					return "detected block transactions hash mismatch";
				case ErrorType::Block_Receipts_Hash:
					return "detected block receipts hash mismatch";

				case ErrorType::Proof_Hash:
					return "detected proof hash mismatch";
				}

				return "undefined error";
			}

		private:
			int m_maxHeightDigits;

			Height m_height;
			std::map<ErrorType, std::vector<std::string>> m_errorMap;
		};

		// endregion

		// region ChainLinkAnalyzer

		class ChainLinkAnalyzer {
		public:
			ChainLinkAnalyzer(const io::BlockStorage& blockStorage, uint64_t importanceGrouping)
					: m_blockStorage(blockStorage)
					, m_importanceGrouping(importanceGrouping)
			{}

		public:
			ErrorAggregator analyze() {
				auto chainHeight = m_blockStorage.chainHeight();

				ErrorAggregator errors;
				errors.setMaxHeight(chainHeight);

				for (auto height = Height(1); height <= chainHeight; height = height + Height(1)) {
					errors.setHeight(height);

					auto pBlockElement = m_blockStorage.loadBlockElement(height);
					errors.requireEqual(ErrorType::Block_Link, m_previousBlockHash, pBlockElement->Block.PreviousBlockHash);
					m_previousBlockHash = pBlockElement->EntityHash;

					auto expectedBlockType = model::CalculateBlockTypeFromHeight(pBlockElement->Block.Height, m_importanceGrouping);
					errors.requireEqual(ErrorType::Block_Type, expectedBlockType, pBlockElement->Block.Type);

					if (model::Entity_Type_Block_Normal != expectedBlockType) {
						const auto& importanceBlockFooter = model::GetBlockFooter<model::ImportanceBlockFooter>(pBlockElement->Block);
						errors.requireEqual(
								ErrorType::Importance_Link,
								m_previousImportanceBlockHash,
								importanceBlockFooter.PreviousImportanceBlockHash);
						m_previousImportanceBlockHash = pBlockElement->EntityHash;
					}
				}

				return errors;
			}

		private:
			const io::BlockStorage& m_blockStorage;
			uint64_t m_importanceGrouping;
			Hash256 m_previousBlockHash;
			Hash256 m_previousImportanceBlockHash;
		};

		// endregion

		// region BlockAnalyzer

		class BlockAnalyzer {
		public:
			BlockAnalyzer(
					const io::BlockStorage& blockStorage,
					const model::TransactionRegistry& transactionRegistry,
					const model::BlockChainConfiguration& config)
					: m_blockStorage(blockStorage)
					, m_transactionRegistry(transactionRegistry)
					, m_config(config)
			{}

		public:
			ErrorAggregator analyze() {
				auto chainHeight = m_blockStorage.chainHeight();

				std::vector<Height> heights;
				for (auto height = Height(1); height <= chainHeight; height = height + Height(1))
					heights.push_back(height);

				uint32_t numWorkerThreads = 2 * std::thread::hardware_concurrency();
				auto pThreadPool = CreateStartedThreadPool(numWorkerThreads);

				std::vector<ErrorAggregator> errorsGroupedByPartition(numWorkerThreads);
				for (auto& partitionErrors : errorsGroupedByPartition)
					partitionErrors.setMaxHeight(chainHeight);

				thread::ParallelForPartition(pThreadPool->ioContext(), heights, numWorkerThreads, [this, &errorsGroupedByPartition](
						auto itBegin,
						auto itEnd,
						auto,
						auto batchIndex) {
					for (auto iter = itBegin; itEnd != iter; ++iter) {
						auto pBlockElement = m_blockStorage.loadBlockElement(*iter);
						analyze(*pBlockElement, errorsGroupedByPartition[batchIndex]);
					}
				});
				pThreadPool->join();

				return ErrorAggregator::Aggregate(std::move(errorsGroupedByPartition));
			}

		private:
			void analyze(const model::BlockElement& blockElement, ErrorAggregator& errors) {
				errors.setHeight(blockElement.Block.Height);

				if (!model::VerifyBlockHeaderSignature(blockElement.Block))
					errors.add(ErrorType::Block_Signature);

				auto calculatedEntityHash = model::CalculateHash(blockElement.Block);
				errors.requireEqual(ErrorType::Block_Entity_Hash, calculatedEntityHash, blockElement.EntityHash);

				Hash256 calculatedTransactionsHash;
				extensions::BlockExtensions blockExtensions(m_config.Network.GenerationHashSeed, m_transactionRegistry);
				blockExtensions.calculateBlockTransactionsHash(blockElement.Block, calculatedTransactionsHash);
				errors.requireEqual(ErrorType::Block_Transactions_Hash, calculatedTransactionsHash, blockElement.Block.TransactionsHash);

				if (m_config.EnableVerifiableReceipts) {
					auto statementData = m_blockStorage.loadBlockStatementData(blockElement.Block.Height).first;
					auto statementDataStream = io::BufferInputStreamAdapter<std::vector<uint8_t>>(statementData);

					model::BlockStatement blockStatement;
					io::ReadBlockStatement(statementDataStream, blockStatement);
					auto calculatedReceiptsHash = model::CalculateMerkleHash(blockStatement);

					errors.requireEqual(ErrorType::Block_Receipts_Hash, calculatedReceiptsHash, blockElement.Block.ReceiptsHash);
				}
			}

		private:
			const io::BlockStorage& m_blockStorage;
			const model::TransactionRegistry& m_transactionRegistry;
			const model::BlockChainConfiguration& m_config;
		};

		// endregion

		// region ProofAnalyzer

		class ProofAnalyzer {
		public:
			ProofAnalyzer(const io::BlockStorage& blockStorage, const io::ProofStorage& proofStorage)
					: m_blockStorage(blockStorage)
					, m_proofStorage(proofStorage)
			{}

		public:
			ErrorAggregator analyze() {
				ErrorAggregator errors;
				errors.setMaxHeight(m_blockStorage.chainHeight());

				auto lastEpoch = m_proofStorage.statistics().Round.Epoch;
				if (FinalizationEpoch(0) == lastEpoch)
					return errors;

				for (auto epoch = FinalizationEpoch(1); epoch <= lastEpoch; epoch = epoch + FinalizationEpoch(1)) {
					auto pProof = m_proofStorage.loadProof(epoch);

					errors.setHeight(pProof->Height);

					auto pBlockElement = m_blockStorage.loadBlockElement(pProof->Height);

					errors.requireEqual(ErrorType::Proof_Hash, pBlockElement->EntityHash, pProof->Hash);
				}

				return errors;
			}

		private:
			const io::BlockStorage& m_blockStorage;
			const io::ProofStorage& m_proofStorage;
		};

		// endregion

		// region VerifyTool

		class VerifyTool : public Tool {
		public:
			std::string name() const override {
				return "Verify Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				AddResourcesOption(optionsBuilder);
			}

			int run(const Options& options) override {
				auto resourcesPath = GetResourcesOptionValue(options);
				auto config = LoadConfiguration(resourcesPath);

				plugins::PluginLoader pluginLoader(config, plugins::CacheDatabaseCleanupMode::None);
				pluginLoader.loadAll();

				auto dataDirectory = (std::filesystem::path(resourcesPath) / config.User.DataDirectory).generic_string();
				io::FileBlockStorage blockStorage(dataDirectory, config.Node.FileDatabaseBatchSize);
				io::FileProofStorage proofStorage(dataDirectory, config.Node.FileDatabaseBatchSize);

				CATAPULT_LOG(important)
						<< "processing chain with " << blockStorage.chainHeight() << " block(s)"
						<< " and " << proofStorage.statistics().Round.Epoch << " proof(s) "
						<< " from " << dataDirectory;

				if (!RunAnalyzer<ChainLinkAnalyzer>("chain link", blockStorage, config.BlockChain.ImportanceGrouping))
					return 1;

				if (!RunAnalyzer<BlockAnalyzer>("block", blockStorage, pluginLoader.transactionRegistry(), config.BlockChain))
					return 2;

				if (!RunAnalyzer<ProofAnalyzer>("proof", blockStorage, proofStorage))
					return 3;

				return 0;
			}

		private:
			template<typename TAnalyzer, typename... TArgs>
			static bool RunAnalyzer(const std::string& name, TArgs&&... args) {
				CATAPULT_LOG(info) << "analyzing " << name;

				TAnalyzer analyzer(std::forward<TArgs>(args)...);
				auto errors = analyzer.analyze();
				if (errors.empty()) {
					CATAPULT_LOG(important) << name << " analysis PASSED";
					return true;
				}

				CATAPULT_LOG(error) << errors.str();
				return false;
			}
		};

		// endregion
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::verify::VerifyTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
