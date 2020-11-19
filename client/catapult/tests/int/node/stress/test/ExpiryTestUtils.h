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
#include "Accounts.h"
#include "BlockChainBuilder.h"
#include "TransactionsBuilder.h"
#include "tests/int/node/test/LocalNodeRequestTestUtils.h"

namespace catapult { namespace test {

	/// Unzips a vector of \a pairs into a pair of vectors.
	template<typename T>
	std::pair<std::vector<T>, std::vector<T>> Unzip(const std::vector<std::pair<T, T>>& pairs) {
		std::pair<std::vector<T>, std::vector<T>> result;
		for (const auto& pair : pairs) {
			result.first.emplace_back(pair.first);
			result.second.emplace_back(pair.second);
		}

		return result;
	}

	/// Result of PushTransferBlocks.
	struct TransferBlocksResult {
		/// Terminal builder.
		BlockChainBuilder Builder;

		/// All pushed blocks.
		BlockChainBuilder::Blocks AllBlocks;

		/// Number of pushed chain parts.
		uint32_t NumAliveChains;
	};

	/// Pushes \a numTotalBlocks blocks to the network using \a context, \a connection, \a accounts and \a builder.
	template<typename TTestContext>
	TransferBlocksResult PushTransferBlocks(
			TTestContext& context,
			ExternalSourceConnection& connection,
			const Accounts& accounts,
			BlockChainBuilder& builder,
			size_t numTotalBlocks) {
		BlockChainBuilder::Blocks allBlocks;
		auto numAliveChains = 0u;
		auto numRemainingBlocks = numTotalBlocks;
		for (;;) {
			auto numBlocks = std::min<size_t>(50, numRemainingBlocks);
			TransactionsBuilder transactionsBuilder(accounts);
			for (auto i = 0u; i < numBlocks; ++i)
				transactionsBuilder.addTransfer(0, 1, Amount(1));

			auto blocks = builder.asBlockChain(transactionsBuilder);
			auto pIo = PushEntities(connection, ionet::PacketType::Push_Block, blocks);

			numRemainingBlocks -= numBlocks;
			++numAliveChains;
			allBlocks.insert(allBlocks.end(), blocks.cbegin(), blocks.cend());

			WaitForHeightAndElements(context, Height(2 + numTotalBlocks - numRemainingBlocks), 1 + numAliveChains, 1);
			if (0 == numRemainingBlocks)
				break;

			builder = builder.createChainedBuilder();
		}

		return TransferBlocksResult{ builder, allBlocks, numAliveChains };
	}
}}
