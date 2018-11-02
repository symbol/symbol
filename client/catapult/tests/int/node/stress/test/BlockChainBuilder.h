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

#pragma once
#include "Accounts.h"
#include "StateHashCalculator.h"
#include "catapult/model/BlockUtils.h"
#include "catapult/state/BlockDifficultyInfo.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include <set>

namespace catapult { namespace test {

	/// Single use builder used to build a single block or block chain from transactions.
	/// \note
	/// transfers are supported by both asSingleBlock and asBlockChain
	/// root namespace registrations are only supporte by asSingleBlock
	class BlockChainBuilder {
	public:
		using Blocks = std::vector<std::shared_ptr<model::Block>>;

	public:
		// region descriptors

		/// Describes a transfer.
		struct TransferDescriptor {
			/// Sender identifier.
			size_t SenderId;
			/// Receipient identifier.
			size_t RecipientId;
			/// Transfer amount.
			catapult::Amount Amount;
		};

		/// Describes a namespace registration.
		struct NamespaceDescriptor {
			/// Owner identifier.
			size_t OwnerId;
			/// Namespace name.
			std::string Name;
			/// Namespace duration.
			BlockDuration Duration;
		};

		// endregion

	public:
		/// Creates a builder around \a accounts and \a stateHashCalculator.
		BlockChainBuilder(const Accounts& accounts, StateHashCalculator& stateHashCalculator);

		/// Creates a builder around \a accounts, \a stateHashCalculator and \a config.
		BlockChainBuilder(
				const Accounts& accounts,
				StateHashCalculator& stateHashCalculator,
				const model::BlockChainConfiguration& config);

	private:
		BlockChainBuilder(
				const Accounts& accounts,
				StateHashCalculator& stateHashCalculator,
				const model::BlockChainConfiguration& config,
				bool isChained);

	public:
		/// Adds a transfer from \a senderId to \a recipientId for amount \a transferAmount.
		void addTransfer(size_t senderId, size_t recipientId, Amount transferAmount);

		/// Adds a root namespace registration for namespace \a name by \a ownerId for specified \a duration.
		void addNamespace(size_t ownerId, const std::string& name, BlockDuration duration);

		/// Sets the time between blocks to \a blockTimeInterval.
		void setBlockTimeInterval(Timestamp blockTimeInterval);

		/// Creates a new builder starting at this builder's terminal block.
		BlockChainBuilder createChainedBuilder();

		/// Creates a new builder starting at this builder's terminal block with a different
		/// state hash calculator (\a stateHashCalculator).
		BlockChainBuilder createChainedBuilder(StateHashCalculator& stateHashCalculator);

	public:
		/// Builds a single block.
		std::unique_ptr<model::Block> asSingleBlock();

		/// Builds a block chain.
		Blocks asBlockChain();

	private:
		void pushDifficulty(const model::Block& block);

		std::unique_ptr<model::Transaction> createTransfer(const TransferDescriptor& descriptor, Timestamp deadline);

		std::unique_ptr<model::Transaction> createRegisterNamespace(const NamespaceDescriptor& descriptor, Timestamp deadline);

		std::unique_ptr<model::Block> createBlock(
				const model::PreviousBlockContext& context,
				Timestamp timestamp,
				const model::Transactions& transactions);

		crypto::KeyPair findSigner(const model::PreviousBlockContext& context, Timestamp timestamp, Difficulty difficulty);

	private:
		static std::shared_ptr<const model::BlockElement> ToSharedBlockElement(
				const Hash256& parentGenerationHash,
				const model::Block& block);

		static std::unique_ptr<model::Transaction> SignWithDeadline(
				std::unique_ptr<model::Transaction>&& pTransaction,
				const crypto::KeyPair& signerKeyPair,
				Timestamp deadline);

	private:
		// pointers instead of references to allow copy
		const Accounts* m_pAccounts;
		StateHashCalculator* m_pStateHashCalculator;

		std::shared_ptr<const model::BlockElement> m_pParentBlockElement;
		std::shared_ptr<const model::BlockElement> m_pTailBlockElement;
		std::shared_ptr<const model::Block> m_pNemesisBlock; // only used to extend block lifetime
		std::set<state::BlockDifficultyInfo> m_difficulties;

		std::vector<TransferDescriptor> m_transferDescriptors;
		std::vector<NamespaceDescriptor> m_namespaceDescriptors;
		Timestamp m_blockTimeInterval;
		model::BlockChainConfiguration m_config;
	};
}}
