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

#include "BlockGenerator.h"
#include "NemesisConfiguration.h"
#include "TransactionRegistryFactory.h"
#include "catapult/builders/MosaicDefinitionBuilder.h"
#include "catapult/builders/MosaicSupplyChangeBuilder.h"
#include "catapult/builders/RegisterNamespaceBuilder.h"
#include "catapult/builders/TransferBuilder.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/extensions/BlockExtensions.h"
#include "catapult/extensions/ConversionExtensions.h"
#include "catapult/extensions/TransactionExtensions.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace tools { namespace nemgen {

	namespace {
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
			void addRegisterNamespace(const std::string& namespaceName, BlockDuration duration) {
				builders::RegisterNamespaceBuilder builder(m_networkIdentifier, m_signer.publicKey(), namespaceName);
				builder.setDuration(duration);
				signAndAdd(builder.build());
			}

			void addRegisterNamespace(const std::string& namespaceName, NamespaceId parentId) {
				builders::RegisterNamespaceBuilder builder(m_networkIdentifier, m_signer.publicKey(), namespaceName);
				builder.setParentId(parentId);
				signAndAdd(builder.build());
			}

			void addMosaicDefinition(NamespaceId parentId, const std::string& mosaicName, const model::MosaicProperties& properties) {
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
				auto recipientUnresolvedAddress = extensions::CopyToUnresolvedAddress(recipientAddress);
				builders::TransferBuilder builder(m_networkIdentifier, m_signer.publicKey(), recipientUnresolvedAddress);
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
	}

	std::unique_ptr<model::Block> CreateNemesisBlock(const NemesisConfiguration& config) {
		auto signer = crypto::KeyPair::FromString(config.NemesisSignerPrivateKey);

		NemesisTransactions transactions(config.NetworkIdentifier, signer);

		// - namespace creation
		for (const auto& rootPair : config.RootNamespaces) {
			// - root
			const auto& root = rootPair.second;
			const auto& rootName = config.NamespaceNames.at(root.id());
			auto duration = std::numeric_limits<BlockDuration::ValueType>::max() == root.lifetime().End.unwrap()
					? Eternal_Artifact_Duration
					: BlockDuration((root.lifetime().End - root.lifetime().Start).unwrap());
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
		pBlock->Type = model::Entity_Type_Nemesis_Block;
		extensions::BlockExtensions().signFullBlock(signer, *pBlock);
		return pBlock;
	}

	namespace {
		Hash256 ParseHash(const std::string& hashString) {
			Hash256 hash;
			utils::ParseHexStringIntoContainer(hashString.c_str(), hashString.size(), hash);
			return hash;
		}
	}

	model::BlockElement CreateNemesisBlockElement(const model::Block& block, const NemesisConfiguration& config) {
		auto registry = CreateTransactionRegistry();
		auto generationHash = ParseHash(config.NemesisGenerationHash);
		return extensions::BlockExtensions(registry).convertBlockToBlockElement(block, generationHash);
	}
}}}
