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

#include "PeerLocalNodeTestContext.h"
#include "LocalNodeRequestTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	namespace {
		void AddAdditionalPlugins(config::CatapultConfiguration& config, NonNemesisTransactionPlugins additionalPlugins) {
			auto& plugins = const_cast<model::BlockChainConfiguration&>(config.BlockChain).Plugins;
			if (NonNemesisTransactionPlugins::Lock_Secret == additionalPlugins) {
				plugins.emplace("catapult.plugins.locksecret", utils::ConfigurationBag({{
					"",
					{
						{ "maxSecretLockDuration", "1h" },
						{ "minProofSize", "10" },
						{ "maxProofSize", "1000" }
					}
				}}));
			}

			if (NonNemesisTransactionPlugins::Restriction_Account == additionalPlugins) {
				plugins.emplace("catapult.plugins.restrictionaccount", utils::ConfigurationBag({{
					"",
					{
						{ "maxAccountRestrictionValues", "10" }
					}
				}}));
			}
		}
	}

	PeerLocalNodeTestContext::PeerLocalNodeTestContext(
			NodeFlag nodeFlag,
			NonNemesisTransactionPlugins additionalPlugins,
			const consumer<config::CatapultConfiguration&>& configTransform)
			: m_context(
					nodeFlag | NodeFlag::With_Partner,
					{},
					[additionalPlugins, configTransform](auto& config) {
						AddAdditionalPlugins(config, additionalPlugins);
						configTransform(config);
					},
					"")
	{}

	const Key& PeerLocalNodeTestContext::publicKey() const {
		return m_context.publicKey();
	}

	local::LocalNode& PeerLocalNodeTestContext::localNode() const {
		return m_context.localNode();
	}

	std::string PeerLocalNodeTestContext::dataDirectory() const {
		return m_context.dataDirectory();
	}

	PeerLocalNodeStats PeerLocalNodeTestContext::stats() const {
		return m_context.stats();
	}

	Height PeerLocalNodeTestContext::height() const {
		ExternalSourceConnection connection(publicKey());
		return GetLocalNodeHeightViaApi(connection);
	}

	Height PeerLocalNodeTestContext::loadSavedStateChainHeight() const {
		return m_context.loadSavedStateChainHeight();
	}

	void PeerLocalNodeTestContext::waitForHeight(Height height) const {
		ExternalSourceConnection connection(publicKey());
		WaitForLocalNodeHeight(connection, height);
	}

	config::CatapultConfiguration PeerLocalNodeTestContext::prepareFreshDataDirectory(const std::string& directory) const {
		return m_context.prepareFreshDataDirectory(directory);
	}

	void PeerLocalNodeTestContext::assertSingleReaderConnection() const {
		AssertSingleReaderConnection(stats());
	}

	void PeerLocalNodeTestContext::AssertSingleReaderConnection(const PeerLocalNodeStats& stats) {
		// Assert: the external reader connection is still active
		EXPECT_EQ(1u, stats.NumActiveReaders);
		EXPECT_EQ(1u, stats.NumActiveWriters);
		EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
	}
}}
