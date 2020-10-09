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

#include "FilechainTestUtils.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/nodeps/TestNetworkConstants.h"

namespace catapult { namespace test {

	namespace {
		void SetBlockChainConfiguration(model::BlockChainConfiguration& config, uint32_t maxDifficultyBlocks) {
			config.Plugins.emplace("catapult.plugins.hashcache", utils::ConfigurationBag({ { "", { {} } } }));

			if (maxDifficultyBlocks > 0)
				config.MaxDifficultyBlocks = maxDifficultyBlocks;

			// set the number of rollback blocks to zero to avoid unnecessarily influencing height-dominant tests
			config.MaxRollbackBlocks = 0;
		}
	}

	config::CatapultConfiguration CreateFileChainCatapultConfiguration(uint32_t maxDifficultyBlocks, const std::string& dataDirectory) {
		auto config = test::CreateCatapultConfigurationWithNemesisPluginExtensions(dataDirectory);
		SetBlockChainConfiguration(const_cast<model::BlockChainConfiguration&>(config.BlockChain), maxDifficultyBlocks);
		return config;
	}

	config::CatapultConfiguration CreateStateHashEnabledCatapultConfiguration(const std::string& dataDirectory) {
		auto config = CreateFileChainCatapultConfiguration(0, dataDirectory);
		const_cast<config::NodeConfiguration&>(config.Node).EnableCacheDatabaseStorage = true;
		const_cast<model::BlockChainConfiguration&>(config.BlockChain).EnableVerifiableState = true;
		return config;
	}

	std::vector<crypto::KeyPair> GetNemesisKeyPairs() {
		std::vector<crypto::KeyPair> nemesisKeyPairs;
		for (const auto* pRecipientPrivateKeyString : test::Test_Network_Private_Keys)
			nemesisKeyPairs.push_back(crypto::KeyPair::FromString(pRecipientPrivateKeyString));

		return nemesisKeyPairs;
	}

	BlockWithAttributes CreateBlock(
			const std::vector<crypto::KeyPair>& nemesisKeyPairs,
			const Address& recipientAddress,
			std::mt19937_64& rnd,
			uint64_t height,
			const utils::TimeSpan& timeSpacing) {
		auto numNemesisAccounts = nemesisKeyPairs.size();

		std::uniform_int_distribution<size_t> numTransactionsDistribution(5, 20);
		auto numTransactions = numTransactionsDistribution(rnd);

		BlockWithAttributes blockWithAttributes;
		test::ConstTransactions transactions;
		std::uniform_int_distribution<size_t> accountIndexDistribution(0, numNemesisAccounts - 1);
		for (auto i = 0u; i < numTransactions; ++i) {
			auto senderIndex = accountIndexDistribution(rnd);
			const auto& sender = nemesisKeyPairs[senderIndex];

			std::uniform_int_distribution<Amount::ValueType> amountDistribution(1000, 10 * 1000);
			Amount amount(amountDistribution(rnd) * 1'000'000);
			auto pTransaction = test::CreateUnsignedTransferTransaction(
					sender.publicKey(),
					extensions::CopyToUnresolvedAddress(recipientAddress),
					amount);
			pTransaction->MaxFee = Amount(0);
			transactions.push_back(std::move(pTransaction));

			blockWithAttributes.SenderIds.push_back(senderIndex);
			blockWithAttributes.Amounts.push_back(amount);
		}

		auto harvesterIndex = accountIndexDistribution(rnd);
		auto pBlock = test::GenerateBlockWithTransactions(nemesisKeyPairs[harvesterIndex], transactions);
		pBlock->Height = Height(height);
		pBlock->Difficulty = Difficulty(Difficulty().unwrap() + height);
		pBlock->Timestamp = Timestamp(height * timeSpacing.millis());

		blockWithAttributes.pBlock = std::move(pBlock);
		return blockWithAttributes;
	}
}}
