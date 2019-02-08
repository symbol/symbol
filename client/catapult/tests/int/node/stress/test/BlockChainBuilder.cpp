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

#include "BlockChainBuilder.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "plugins/txes/namespace/src/model/NamespaceIdGenerator.h"
#include "catapult/chain/BlockDifficultyScorer.h"
#include "catapult/chain/BlockScorer.h"
#include "catapult/io/FileBlockStorage.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nodeps/MijinConstants.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
	}

	BlockChainBuilder::BlockChainBuilder(const Accounts& accounts, StateHashCalculator& stateHashCalculator)
			: BlockChainBuilder(accounts, stateHashCalculator, CreateLocalNodeBlockChainConfiguration())
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config)
			: BlockChainBuilder(accounts, stateHashCalculator, config, stateHashCalculator.dataDirectory())
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config,
			const std::string& resourcesPath)
			: BlockChainBuilder(accounts, stateHashCalculator, config, resourcesPath, false)
	{}

	BlockChainBuilder::BlockChainBuilder(
			const Accounts& accounts,
			StateHashCalculator& stateHashCalculator,
			const model::BlockChainConfiguration& config,
			const std::string& resourcesPath,
			bool isChained)
			: m_pAccounts(&accounts)
			, m_pStateHashCalculator(&stateHashCalculator)
			, m_blockTimeInterval(60'000)
			, m_blockReceiptsHashCalculator([](const auto&) { return Hash256(); })
			, m_config(config) {
		if (isChained)
			return;

		// seed the state hash calculator with the nemesis state
		if (resourcesPath.empty()) {
			CATAPULT_LOG(debug) << "initializing BlockChainBuilder from memory";
			mocks::MockMemoryBlockStorage storage;
			m_pParentBlockElement = storage.loadBlockElement(Height(1));

			// extend lifetime of nemesis block (referenced by m_pParentBlockElement) beyond lifetime of storage
			// (FileBlockStorage automatically extends block lifetime)
			m_pNemesisBlock = storage.loadBlock(Height(1));
		} else {
			CATAPULT_LOG(debug) << "initializing BlockChainBuilder from resources path: " << resourcesPath;
			io::FileBlockStorage storage(resourcesPath);
			m_pParentBlockElement = storage.loadBlockElement(Height(1));
		}

		m_pStateHashCalculator->execute(m_pParentBlockElement->Block);
	}

	void BlockChainBuilder::addTransfer(size_t senderId, size_t recipientId, Amount transferAmount) {
		m_transferDescriptors.push_back(TransferDescriptor{ senderId, recipientId, transferAmount, "" });
		m_descriptorOrdering.push_back(DescriptorType::Transfer);
	}

	void BlockChainBuilder::addTransfer(size_t senderId, const std::string& recipientAlias, Amount transferAmount) {
		m_transferDescriptors.push_back(TransferDescriptor{ senderId, 0, transferAmount, recipientAlias });
		m_descriptorOrdering.push_back(DescriptorType::Transfer);
	}

	void BlockChainBuilder::addNamespace(size_t ownerId, const std::string& name, BlockDuration duration, size_t aliasId) {
		m_namespaceDescriptors.push_back(NamespaceDescriptor{ ownerId, name, duration, aliasId });
		m_descriptorOrdering.push_back(DescriptorType::Namespace);
	}

	void BlockChainBuilder::setBlockTimeInterval(Timestamp blockTimeInterval) {
		m_blockTimeInterval = blockTimeInterval;
	}

	void BlockChainBuilder::setBlockReceiptsHashCalculator(const BlockReceiptsHashCalculator& blockReceiptsHashCalculator) {
		m_blockReceiptsHashCalculator = blockReceiptsHashCalculator;
	}

	BlockChainBuilder BlockChainBuilder::createChainedBuilder() {
		return createChainedBuilder(*m_pStateHashCalculator);
	}

	BlockChainBuilder BlockChainBuilder::createChainedBuilder(StateHashCalculator& stateHashCalculator) const {
		// resources directory is not used when creating chained builder
		auto builder = BlockChainBuilder(*m_pAccounts, stateHashCalculator, m_config, "", true);
		builder.m_pParentBlockElement = m_pTailBlockElement;
		builder.m_difficulties = m_difficulties;
		return builder;
	}

	std::unique_ptr<model::Block> BlockChainBuilder::asSingleBlock() {
		model::PreviousBlockContext context(*m_pParentBlockElement);
		pushDifficulty(m_pParentBlockElement->Block);

		size_t transferIndex = 0;
		size_t namespaceIndex = 0;
		model::Transactions transactions;
		auto blockTimestamp = context.Timestamp + m_blockTimeInterval;
		for (auto descriptorType : m_descriptorOrdering) {
			if (DescriptorType::Transfer == descriptorType) {
				transactions.push_back(createTransfer(m_transferDescriptors[transferIndex], blockTimestamp));
				++transferIndex;
			} else {
				const auto& descriptor = m_namespaceDescriptors[namespaceIndex];
				transactions.push_back(createRegisterNamespace(descriptor, blockTimestamp));
				if (0 != descriptor.AddressAliasId)
					transactions.push_back(createAddressAlias(descriptor, blockTimestamp));

				++namespaceIndex;
			}
		}

		auto pBlock = createBlock(context, blockTimestamp, transactions);
		m_pTailBlockElement = ToSharedBlockElement(context.GenerationHash, *pBlock);
		return pBlock;
	}

	BlockChainBuilder::Blocks BlockChainBuilder::asBlockChain() {
		auto pParentBlockElement = m_pParentBlockElement;

		Blocks blocks;
		for (const auto& descriptor : m_transferDescriptors) {
			model::PreviousBlockContext context(*pParentBlockElement);
			pushDifficulty(pParentBlockElement->Block);

			auto blockTimestamp = context.Timestamp + m_blockTimeInterval;
			auto pBlock = createBlock(context, blockTimestamp, { createTransfer(descriptor, blockTimestamp) });
			pParentBlockElement = ToSharedBlockElement(context.GenerationHash, *pBlock);
			blocks.push_back(std::move(pBlock));
		}

		m_pTailBlockElement = pParentBlockElement;
		return blocks;
	}

	void BlockChainBuilder::pushDifficulty(const model::Block& block) {
		m_difficulties.insert(state::BlockDifficultyInfo(block.Height, block.Timestamp, block.Difficulty));

		if (m_difficulties.size() > m_config.MaxDifficultyBlocks)
			m_difficulties.erase(m_difficulties.cbegin());
	}

	namespace {
		UnresolvedAddress RootAliasToAddress(const std::string& namespaceName) {
			auto namespaceId = model::GenerateNamespaceId(NamespaceId(), namespaceName);

			UnresolvedAddress address{}; // force zero initialization
			address[0].Byte = utils::to_underlying_type(Network_Identifier) | 0x01;
			std::memcpy(address.data() + 1, &namespaceId, sizeof(NamespaceId));
			return address;
		}
	}

	std::unique_ptr<model::Transaction> BlockChainBuilder::createTransfer(const TransferDescriptor& descriptor, Timestamp deadline) {
		const auto& senderKeyPair = m_pAccounts->getKeyPair(descriptor.SenderId);
		auto recipientAddress = descriptor.RecipientAlias.empty()
				? extensions::CopyToUnresolvedAddress(m_pAccounts->getAddress(descriptor.RecipientId))
				: RootAliasToAddress(descriptor.RecipientAlias);

		auto pTransaction = CreateTransferTransaction(senderKeyPair, recipientAddress, descriptor.Amount);
		return SignWithDeadline(std::move(pTransaction), senderKeyPair, deadline);
	}

	std::unique_ptr<model::Transaction> BlockChainBuilder::createRegisterNamespace(
			const NamespaceDescriptor& descriptor,
			Timestamp deadline) {
		const auto& ownerKeyPair = m_pAccounts->getKeyPair(descriptor.OwnerId);

		auto pTransaction = CreateRegisterRootNamespaceTransaction(ownerKeyPair, descriptor.Name, descriptor.Duration);
		return SignWithDeadline(std::move(pTransaction), ownerKeyPair, deadline);
	}

	std::unique_ptr<model::Transaction> BlockChainBuilder::createAddressAlias(const NamespaceDescriptor& descriptor, Timestamp deadline) {
		const auto& ownerKeyPair = m_pAccounts->getKeyPair(descriptor.OwnerId);
		const auto& aliasedAddress = m_pAccounts->getAddress(descriptor.AddressAliasId);

		auto pTransaction = CreateRootAddressAliasTransaction(ownerKeyPair, descriptor.Name, aliasedAddress);
		return SignWithDeadline(std::move(pTransaction), ownerKeyPair, deadline);
	}

	std::unique_ptr<model::Block> BlockChainBuilder::createBlock(
			const model::PreviousBlockContext& context,
			Timestamp timestamp,
			const model::Transactions& transactions) {
		auto difficulty = chain::CalculateDifficulty(cache::DifficultyInfoRange(m_difficulties.cbegin(), m_difficulties.cend()), m_config);

		auto signer = findSigner(context, timestamp, difficulty);
		auto pBlock = model::CreateBlock(context, Network_Identifier, signer.publicKey(), transactions);
		pBlock->Timestamp = timestamp;
		pBlock->Difficulty = difficulty;

		pBlock->BlockReceiptsHash = m_blockReceiptsHashCalculator(*pBlock);
		m_pStateHashCalculator->updateStateHash(*pBlock);
		SignBlock(signer, *pBlock);
		return pBlock;
	}

	crypto::KeyPair BlockChainBuilder::findSigner(const model::PreviousBlockContext& context, Timestamp timestamp, Difficulty difficulty) {
		chain::BlockHitPredicate hitPredicate(m_config, [](const auto& publicKey, auto) {
			// to simplfy tests, just return nemesis importance
			return GetNemesisImportance(publicKey);
		});

		auto i = 0u;
		for (const auto* pPrivateKeyString : Mijin_Test_Private_Keys) {
			// skip first test account because it is used to fund other accounts
			if (0u == i++)
				continue;

			auto keyPair = crypto::KeyPair::FromString(pPrivateKeyString);

			chain::BlockHitContext blockHitContext;
			blockHitContext.GenerationHash = model::CalculateGenerationHash(context.GenerationHash, keyPair.publicKey());
			blockHitContext.ElapsedTime = utils::TimeSpan::FromDifference(timestamp, context.Timestamp);
			blockHitContext.Signer = keyPair.publicKey();
			blockHitContext.Difficulty = difficulty;
			blockHitContext.Height = context.BlockHeight + Height(1);

			if (hitPredicate(blockHitContext))
				return keyPair;
		}

		CATAPULT_THROW_RUNTIME_ERROR("no eligible harvesting accounts were found");
	}

	std::shared_ptr<const model::BlockElement> BlockChainBuilder::ToSharedBlockElement(
			const Hash256& parentGenerationHash,
			const model::Block& block) {
		auto pBlockElement = std::make_shared<model::BlockElement>(BlockToBlockElement(block));
		pBlockElement->GenerationHash = model::CalculateGenerationHash(parentGenerationHash, block.Signer);
		return std::move(pBlockElement);
	}

	std::unique_ptr<model::Transaction> BlockChainBuilder::SignWithDeadline(
			std::unique_ptr<model::Transaction>&& pTransaction,
			const crypto::KeyPair& signerKeyPair,
			Timestamp deadline) {
		pTransaction->Deadline = deadline;
		pTransaction->MaxFee = Amount(0);
		extensions::SignTransaction(signerKeyPair, *pTransaction);
		return std::move(pTransaction);
	}
}}
