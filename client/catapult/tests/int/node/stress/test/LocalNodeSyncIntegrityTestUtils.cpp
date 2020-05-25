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

#include "LocalNodeSyncIntegrityTestUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "tests/test/nodeps/TestConstants.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region test context

	namespace {
		constexpr auto State_Hash_Directory = "statehash";
	}

	StateHashDisabledTestContext::StateHashDisabledTestContext(
			NonNemesisTransactionPlugins additionalPlugins,
			const consumer<config::CatapultConfiguration&>& configTransform)
			: PeerLocalNodeTestContext(NodeFlag::Regular, additionalPlugins, configTransform)
	{}

	StateHashCalculator StateHashDisabledTestContext::createStateHashCalculator() const {
		return StateHashCalculator();
	}

	StateHashEnabledTestContext::StateHashEnabledTestContext(
			NonNemesisTransactionPlugins additionalPlugins,
			const consumer<config::CatapultConfiguration&>& configTransform)
			: PeerLocalNodeTestContext(NodeFlag::Verify_State, additionalPlugins, configTransform)
			, m_stateHashCalculationDir(State_Hash_Directory) // isolated directory used for state hash calculation
	{}

	StateHashCalculator StateHashEnabledTestContext::createStateHashCalculator() const {
		{
			TempDirectoryGuard forceCleanResourcesDir(State_Hash_Directory);
		}

		return StateHashCalculator(prepareFreshDataDirectory(m_stateHashCalculationDir.name()));
	}

	// endregion

	// region non-assert test utils

	Hash256 GetStateHash(const PeerLocalNodeTestContext& context) {
		return context.localNode().cache().createView().calculateStateHash().StateHash;
	}

	void SeedStateHashCalculator(StateHashCalculator& stateHashCalculator, const BlockChainBuilder::Blocks& blocks) {
		// can load nemesis from memory because it is only used for execution, so state hash can be wrong
		mocks::MockMemoryBlockStorage storage;
		auto pNemesisBlockElement = storage.loadBlockElement(Height(1));
		stateHashCalculator.execute(pNemesisBlockElement->Block);

		for (const auto& pBlock : blocks)
			stateHashCalculator.execute(*pBlock);
	}

	void WaitForHeightAndElements(
			const PeerLocalNodeTestContext& context,
			Height height,
			uint32_t numExpectedBlockElements,
			uint32_t numTerminalReaders) {
		// Act: wait for the chain height to change, for all height readers to disconnect and completion of block element processing
		context.waitForHeight(height);
		auto chainHeight = context.height();
		WAIT_FOR_VALUE_EXPR(numTerminalReaders, context.stats().NumActiveReaders);
		WAIT_FOR_ZERO_EXPR(context.stats().NumActiveBlockElements);

		// Assert: the chain height is correct
		EXPECT_EQ(height, chainHeight);

		// - the expected number of block elements were added
		auto stats = context.stats();
		EXPECT_EQ(numExpectedBlockElements, stats.NumAddedBlockElements);
		EXPECT_EQ(0u, stats.NumAddedTransactionElements);

		// - the connection is still active
		EXPECT_EQ(numTerminalReaders, stats.NumActiveReaders);
	}

	// endregion

	// region state hash asserts

	void AssertAllZero(const std::vector<Hash256>& hashes, size_t numExpected, const std::string& message) {
		// Sanity:
		EXPECT_EQ(numExpected, hashes.size());

		// Assert:
		auto i = 0u;
		for (const auto& hash : hashes) {
			EXPECT_EQ(Hash256(), hash) << "hash at " << i << " " << message;
			++i;
		}
	}

	void AssertAllZero(const std::pair<std::vector<Hash256>, std::vector<Hash256>>& hashes, size_t numExpected) {
		// Assert:
		AssertAllZero(hashes.first, numExpected, "first");
		AssertAllZero(hashes.second, numExpected, "second");
	}

	void AssertAllNonzero(const std::vector<Hash256>& hashes, size_t numExpected, const std::string& message) {
		// Sanity:
		EXPECT_EQ(numExpected, hashes.size());

		// Assert:
		auto i = 0u;
		for (const auto& hash : hashes) {
			EXPECT_NE(Hash256(), hash) << "hash at " << i << " " << message;
			++i;
		}
	}

	void AssertUnique(const std::vector<Hash256>& hashes) {
		EXPECT_EQ(hashes.size(), utils::HashSet(hashes.cbegin(), hashes.cend()).size());
	}

	// endregion

	// region state asserts

	void AssertCurrencyBalances(
			const Accounts& accounts,
			const cache::CatapultCache& cache,
			const std::vector<ExpectedBalance>& expectedBalances) {
		auto cacheView = cache.createView();
		const auto& accountStateCache = cacheView.sub<cache::AccountStateCache>();

		for (const auto& expectedBalance : expectedBalances) {
			const auto& address = accounts.getAddress(expectedBalance.AccountShortId);

			std::ostringstream out;
			out << "account id " << expectedBalance.AccountShortId << " (" << model::AddressToString(address) << ")";
			auto message = out.str();

			// Assert:
			auto accountStateIter = accountStateCache.find(address);
			EXPECT_TRUE(!!accountStateIter.tryGet()) << message;
			if (!accountStateIter.tryGet())
				continue;

			const auto& accountState = accountStateIter.get();
			EXPECT_EQ(1u, accountState.Balances.size()) << message;
			EXPECT_EQ(expectedBalance.Balance, accountState.Balances.get(Default_Currency_Mosaic_Id)) << message;
		}
	}

	void AssertNamespaceCount(const local::LocalNode& localNode, size_t numExpectedNamespaces) {
		// Assert:
		auto numNamespaces = GetCounterValue(localNode.counters(), "NS C");
		auto numActiveNamespaces = GetCounterValue(localNode.counters(), "NS C AS");
		EXPECT_EQ(numExpectedNamespaces, numNamespaces);
		EXPECT_EQ(numExpectedNamespaces + 2, numActiveNamespaces); // default namespace defined in nemesis has two subnamespaces
	}

	// endregion

}}
