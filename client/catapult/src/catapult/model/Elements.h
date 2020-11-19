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

#pragma once
#include "Block.h"
#include "BlockStatement.h"
#include "ContainerTypes.h"
#include "EntityInfo.h"
#include "WeakEntityInfo.h"
#include "catapult/functions.h"
#include <unordered_set>

namespace catapult { namespace model {

	/// Processing element for a transaction composed of a transaction and metadata.
	struct TransactionElement {
	public:
		/// Creates a transaction element around \a transaction.
		explicit TransactionElement(const model::Transaction& transaction) : Transaction(transaction)
		{}

	public:
		/// Transaction entity.
		const model::Transaction& Transaction;

		/// Entity hash.
		Hash256 EntityHash;

		/// Modified hash that should be used as a hash in the merkle tree.
		Hash256 MerkleComponentHash;

		/// Optional extracted addresses.
		/// \note shared_ptr for optionality and more performant copyability.
		std::shared_ptr<const UnresolvedAddressSet> OptionalExtractedAddresses;
	};

	/// Processing element for a block composed of a block and metadata.
	struct BlockElement {
	public:
		/// Creates a block element around \a block.
		explicit BlockElement(const model::Block& block) : Block(block)
		{}

	public:
		/// Block entity.
		const model::Block& Block;

		/// Entity hash.
		Hash256 EntityHash;

		/// Generation hash of the block.
		catapult::GenerationHash GenerationHash;

		/// Merkle roots for all sub caches at the current block.
		std::vector<Hash256> SubCacheMerkleRoots;

		/// Transaction elements.
		std::vector<TransactionElement> Transactions;

		/// Optional block statement.
		/// \note shared_ptr for optionality and copyability (BlockStatement is move only).
		std::shared_ptr<const BlockStatement> OptionalStatement;
	};

	/// Predicate for evaluating a timestamp, a hash and an entity type.
	using MatchingEntityPredicate = predicate<BasicEntityType, Timestamp, const Hash256&>;

	/// Extracts all entity infos for which \a predicate returns \c true from \a elements into \a entityInfos.
	void ExtractMatchingEntityInfos(
			const std::vector<BlockElement>& elements,
			WeakEntityInfos& entityInfos,
			const MatchingEntityPredicate& predicate);

	/// Extracts all entity infos from \a element into \a entityInfos.
	void ExtractEntityInfos(const BlockElement& element, WeakEntityInfos& entityInfos);

	/// Extracts transaction infos from a block element (\a pBlockElement) into \a transactionInfos
	/// such that each transaction will extend the lifetime of the owning block element.
	void ExtractTransactionInfos(std::vector<TransactionInfo>& transactionInfos, const std::shared_ptr<const BlockElement>& pBlockElement);

	/// Makes a transaction info by merging \a pTransaction and \a transactionElement.
	TransactionInfo MakeTransactionInfo(
			const std::shared_ptr<const Transaction>& pTransaction,
			const TransactionElement& transactionElement);
}}
