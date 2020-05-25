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
#include "BlockChainBuilder.h"
#include "StateHashCalculator.h"
#include "tests/int/node/test/PeerLocalNodeTestContext.h"
#include "tests/test/nodeps/Filesystem.h"

namespace catapult { namespace test {

	// region test context

	/// Test context for running tests with state hash disabled.
	class StateHashDisabledTestContext : public PeerLocalNodeTestContext {
	public:
		/// Creates a test context around \a additionalPlugins and \a configTransform.
		explicit StateHashDisabledTestContext(
				NonNemesisTransactionPlugins additionalPlugins = NonNemesisTransactionPlugins::None,
				const consumer<config::CatapultConfiguration&>& configTransform = [](const auto&) {});

	public:
		/// Creates a state hash calculator.
		StateHashCalculator createStateHashCalculator() const;
	};

	/// Test context for running tests with state hash enabled.
	class StateHashEnabledTestContext : public PeerLocalNodeTestContext {
	public:
		/// Creates a test context around \a additionalPlugins and \a configTransform.
		explicit StateHashEnabledTestContext(
				NonNemesisTransactionPlugins additionalPlugins = NonNemesisTransactionPlugins::None,
				const consumer<config::CatapultConfiguration&>& configTransform = [](const auto&) {});

	public:
		/// Creates a state hash calculator.
		StateHashCalculator createStateHashCalculator() const;

	private:
		TempDirectoryGuard m_stateHashCalculationDir;
	};

	// endregion

	// region non-assert test utils

	/// Gets the state hash corresponding to the local node wrapped by \a context.
	Hash256 GetStateHash(const PeerLocalNodeTestContext& context);

	/// Seeds \a stateHashCalculator with \a blocks.
	void SeedStateHashCalculator(StateHashCalculator& stateHashCalculator, const BlockChainBuilder::Blocks& blocks);

	/// Waits for the local node wrapped by \a context to reach \a height with \a numExpectedBlockElements block elements
	/// and \a numTerminalReaders terminal readers.
	void WaitForHeightAndElements(
			const PeerLocalNodeTestContext& context,
			Height height,
			uint32_t numExpectedBlockElements,
			uint32_t numTerminalReaders);

	// endregion

	// region state hash asserts

	/// Asserts that every hash in \a hashes is zero and \a numExpected hashes are provided with optional \a message.
	void AssertAllZero(const std::vector<Hash256>& hashes, size_t numExpected, const std::string& message = "");

	/// Asserts that every hash in \a hashesPair is zero and \a numExpected hashes are provided in each part.
	void AssertAllZero(const std::pair<std::vector<Hash256>, std::vector<Hash256>>& hashes, size_t numExpected);

	/// Asserts that every hash in \a hashes is nonzero and \a numExpected hashes are provided with optional \a message.
	void AssertAllNonzero(const std::vector<Hash256>& hashes, size_t numExpected, const std::string& message = "");

	/// Asserts that every hash in \a hashes is unique.
	void AssertUnique(const std::vector<Hash256>& hashes);

	// endregion

	// region state asserts

	/// Expected balance.
	struct ExpectedBalance {
		/// Account identifier.
		size_t AccountShortId;

		/// Expected account balance.
		Amount Balance;
	};

	/// Asserts that \a cache has currency balances as specified in \a expectedBalances using
	/// \a accounts to convert account identifiers to addresses.
	void AssertCurrencyBalances(
			const Accounts& accounts,
			const cache::CatapultCache& cache,
			const std::vector<ExpectedBalance>& expectedBalances);

	/// Asserts that \a localNode has \a numExpectedNamespaces namespace cache entries.
	void AssertNamespaceCount(const local::LocalNode& localNode, size_t numExpectedNamespaces);

	// endregion
}}
