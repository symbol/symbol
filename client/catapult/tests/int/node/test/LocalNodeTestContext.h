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
#include "LocalNodeStateHashTestUtils.h"
#include "LocalNodeTestUtils.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/extensions/ServiceState.h"
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
				: LocalNodeTestContext(nodeFlag, nodes, [](const auto&) {}, "")
		{}

		/// Creates a context around \a nodeFlag with custom \a nodes, config transform (\a configTransform)
		/// and temp directory postfix (\a tempDirPostfix).
		LocalNodeTestContext(
				NodeFlag nodeFlag,
				const std::vector<ionet::Node>& nodes,
				const consumer<config::LocalNodeConfiguration&>& configTransform,
				const std::string& tempDirPostfix)
				: m_nodeFlag(nodeFlag)
				, m_nodes(nodes)
				, m_configTransform(configTransform)
				, m_serverKeyPair(loadServerKeyPair())
				, m_partnerServerKeyPair(LoadPartnerServerKeyPair())
				, m_numPreLoadHandlerCalls(0)
				, m_tempDir("../temp/lntc" + tempDirPostfix)
				, m_partnerTempDir(m_tempDir.name() + "_partner") {
			initializeDataDirectory(m_tempDir.name());

			if (HasFlag(NodeFlag::With_Partner, nodeFlag)) {
				initializeDataDirectory(m_partnerTempDir.name());

				m_pLocalPartnerNode = BootLocalPartnerNode(m_partnerTempDir.name(), m_partnerServerKeyPair, nodeFlag);
			}

			if (!HasFlag(NodeFlag::Require_Explicit_Boot, nodeFlag))
				boot();
		}

	private:
		void initializeDataDirectory(const std::string& directory) const {
			PrepareStorage(directory);
			PrepareConfiguration(directory, m_nodeFlag);

			if (HasFlag(NodeFlag::Verify_State, m_nodeFlag)) {
				auto config = CreateLocalNodeConfiguration(directory);
				prepareLocalNodeConfiguration(config);

				SetNemesisStateHash(directory, config);
			}
		}

	public:
		/// Gets the resources directory.
		std::string resourcesDirectory() const {
			return m_tempDir.name() + "/resources";
		}

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
		config::LocalNodeConfiguration createConfig() const {
			return CreateLocalNodeConfiguration(m_tempDir.name());
		}

		/// Prepares a fresh data \a directory and returns corresponding configuration.
		config::LocalNodeConfiguration prepareFreshDataDirectory(const std::string& directory) const {
			initializeDataDirectory(directory);

			auto config = CreateLocalNodeConfiguration(directory);
			prepareLocalNodeConfiguration(config);
			return config;
		}

	public:
		/// Boots a new local node.
		/// \note This overload is intended to be called only for nodes that require explicit booting.
		void boot() {
			if (m_pLocalNode)
				CATAPULT_THROW_RUNTIME_ERROR("cannot boot local node multiple times via same test context");

			m_pLocalNode = boot(createConfig());
		}

		/// Boots a new local node around \a config.
		std::unique_ptr<local::BootedLocalNode> boot(config::LocalNodeConfiguration&& config) const {
			prepareLocalNodeConfiguration(config);

			auto pBootstrapper = std::make_unique<extensions::LocalNodeBootstrapper>(
					std::move(config),
					resourcesDirectory(),
					"LocalNodeTests");
			pBootstrapper->addStaticNodes(m_nodes);

			auto& extensionManager = pBootstrapper->extensionManager();
			if (TTraits::ShouldRegisterPreLoadHandler) {
				auto& counter = const_cast<uint32_t&>(m_numPreLoadHandlerCalls);
				extensionManager.addPreLoadHandler([&counter](const auto&) { ++counter; });
			}

			extensionManager.addServiceRegistrar(std::make_unique<CapturingServiceRegistrar>(m_capturedServiceState));
			pBootstrapper->loadExtensions();

			return local::CreateBasicLocalNode(m_serverKeyPair, std::move(pBootstrapper));
		}

	private:
		void prepareLocalNodeConfiguration(config::LocalNodeConfiguration& config) const {
			PrepareLocalNodeConfiguration(config, TTraits::AddPluginExtensions, m_nodeFlag);
			m_configTransform(config);
		}

		crypto::KeyPair loadServerKeyPair() const {
			// can pass empty string to CreateLocalNodeConfiguration because this config is only being used to get boot key
			auto config = CreateLocalNodeConfiguration("");
			m_configTransform(config);
			return crypto::KeyPair::FromString(config.User.BootKey);
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

	public:
		/// Gets the captured node subscriber.
		subscribers::NodeSubscriber& nodeSubscriber() const {
			return *m_capturedServiceState.pNodeSubscriber;
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
		struct CapturedServiceState {
			subscribers::NodeSubscriber* pNodeSubscriber;
		};

		// service registrar for capturing ServiceState values
		// \note only node subscriber is currently captured
		class CapturingServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit CapturingServiceRegistrar(CapturedServiceState& state) : m_state(state)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Capturing", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// do nothing
			}

			void registerServices(extensions::ServiceLocator&, extensions::ServiceState& state) override {
				m_state.pNodeSubscriber = &state.nodeSubscriber();
			}

		private:
			CapturedServiceState& m_state;
		};

	private:
		NodeFlag m_nodeFlag;
		std::vector<ionet::Node> m_nodes;
		consumer<config::LocalNodeConfiguration&> m_configTransform;
		crypto::KeyPair m_serverKeyPair;
		crypto::KeyPair m_partnerServerKeyPair;
		uint32_t m_numPreLoadHandlerCalls;
		TempDirectoryGuard m_tempDir;
		TempDirectoryGuard m_partnerTempDir;

		std::unique_ptr<local::BootedLocalNode> m_pLocalPartnerNode;
		std::unique_ptr<local::BootedLocalNode> m_pLocalNode;
		mutable CapturedServiceState m_capturedServiceState;
	};
}}
