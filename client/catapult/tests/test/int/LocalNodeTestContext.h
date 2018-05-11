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
#include "ConfigurationTestUtils.h"
#include "LocalNodeTestUtils.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/local/BasicLocalNode.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/nodeps/Filesystem.h"

namespace catapult { namespace test {

	/// Traits for a peer node.
	struct LocalNodePeerTraits {
		static constexpr auto CountersToLocalNodeStats = test::CountersToPeerLocalNodeStats;
		static constexpr auto AddPluginExtensions = test::AddPeerPluginExtensions;
		static constexpr auto ShouldRegisterPreLoadHandler = false;
	};

	/// Traits for an api node.
	struct LocalNodeApiTraits {
		static constexpr auto CountersToLocalNodeStats = test::CountersToBasicLocalNodeStats;
		static constexpr auto AddPluginExtensions = test::AddApiPluginExtensions;
		static constexpr auto ShouldRegisterPreLoadHandler = true;
	};

	/// A common test context for local node tests.
	template<typename TTraits>
	class LocalNodeTestContext {
	public:
		/// Creates a context around \a nodeFlag.
		explicit LocalNodeTestContext(NodeFlag nodeFlag) : LocalNodeTestContext(nodeFlag, {})
		{}

		/// Creates a context around \a nodeFlag with custom \a nodes.
		LocalNodeTestContext(NodeFlag nodeFlag, const std::vector<ionet::Node>& nodes)
				: m_nodes(nodes)
				, m_partnerServerKeyPair(LoadPartnerServerKeyPair())
				, m_serverKeyPair(LoadServerKeyPair())
				, m_numPreLoadHandlerCalls(0) {
			PrepareStorage(m_tempDir.name());
			PrepareConfiguration(m_tempDir.name(), nodeFlag);

			if (HasFlag(NodeFlag::With_Partner, nodeFlag))
				m_pLocalPartnerNode = BootLocalPartnerNode(m_tempDir.name(), m_partnerServerKeyPair);

			m_pLocalNode = boot(createConfig());
		}

	public:
		/// Gets the primary (first) booted local node.
		local::BootedLocalNode& localNode() const {
			return *m_pLocalNode;
		}

		/// Gets the node stats.
		auto stats() const {
			return TTraits::CountersToLocalNodeStats(m_pLocalNode->counters());
		}

	public:
		/// Creates a copy of the default local node configuration.
		config::LocalNodeConfiguration createConfig() {
			return LoadLocalNodeConfiguration(m_tempDir.name());
		}

		/// Boots a new local node around \a config.
		std::unique_ptr<local::BootedLocalNode> boot(config::LocalNodeConfiguration&& config) const {
			// in order for the nemesis block to be processed, at least the transfer plugin needs to be loaded
			AddNemesisPluginExtensions(const_cast<model::BlockChainConfiguration&>(config.BlockChain));
			TTraits::AddPluginExtensions(const_cast<config::NodeConfiguration&>(config.Node));

			auto pBootstrapper = std::make_unique<extensions::LocalNodeBootstrapper>(
					std::move(config),
					m_tempDir.name() + "/resources",
					"LocalNodeTests");
			pBootstrapper->addStaticNodes(m_nodes);

			if (TTraits::ShouldRegisterPreLoadHandler) {
				auto& counter = const_cast<uint32_t&>(m_numPreLoadHandlerCalls);
				pBootstrapper->extensionManager().addPreLoadHandler([&counter](const auto&) { ++counter; });
			}

			pBootstrapper->loadExtensions();

			return local::CreateBasicLocalNode(m_serverKeyPair, std::move(pBootstrapper));
		}

	public:
		/// Waits for \a value active readers.
		void waitForNumActiveReaders(size_t value) const {
			WAIT_FOR_VALUE_EXPR(value, stats().NumActiveReaders);
		}

		/// Waits for \a value active writers.
		void waitForNumActiveWriters(size_t value) const {
			WAIT_FOR_VALUE_EXPR(value, stats().NumActiveWriters);
		}

		/// Waits for \a value scheduled tasks.
		void waitForNumScheduledTasks(size_t value) const {
			WAIT_FOR_VALUE_EXPR(value, stats().NumScheduledTasks);
		}

	protected:
		/// Gets the number of times the pre load handler was called.
		uint32_t numPreLoadHandlerCalls() const {
			return m_numPreLoadHandlerCalls;
		}

		/// Gets the supplemental state path.
		boost::filesystem::path getSupplementalStatePath() const {
			return boost::filesystem::path(m_tempDir.name()) / "state" / "supplemental.dat";
		}

	private:
		std::vector<ionet::Node> m_nodes;
		crypto::KeyPair m_partnerServerKeyPair;
		crypto::KeyPair m_serverKeyPair;
		uint32_t m_numPreLoadHandlerCalls;
		TempDirectoryGuard m_tempDir;

		std::unique_ptr<local::BootedLocalNode> m_pLocalPartnerNode;
		std::unique_ptr<local::BootedLocalNode> m_pLocalNode;
	};
}}
