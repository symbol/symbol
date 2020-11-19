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
#include "ResolutionStatement.h"
#include "TransactionStatement.h"
#include <map>

namespace catapult { namespace model {

	/// Collection of statements scoped to a block.
	struct BlockStatement {
		/// Transaction statements.
		std::map<ReceiptSource, TransactionStatement> TransactionStatements;

		/// Address resolution statements.
		std::map<UnresolvedAddress, AddressResolutionStatement> AddressResolutionStatements;

		/// Mosaic resolution statements.
		std::map<UnresolvedMosaicId, MosaicResolutionStatement> MosaicResolutionStatements;
	};

	/// Calculates the merkle hash for \a statement.
	Hash256 CalculateMerkleHash(const BlockStatement& statement);

	/// Calculates the merkle tree for \a statement.
	std::vector<Hash256> CalculateMerkleTree(const BlockStatement& statement);

	/// Counts the total number of statements in \a statement.
	size_t CountTotalStatements(const BlockStatement& statement);

	/// Creates a deep copy of \a source into \a destination.
	void DeepCopyTo(BlockStatement& destination, const BlockStatement& source);

	/// Creates a deep copy of \a source into \a destination excluding receipts with primary source id greater than \a maxSourcePrimaryId.
	void DeepCopyTo(BlockStatement& destination, const BlockStatement& source, uint32_t maxSourcePrimaryId);
}}
