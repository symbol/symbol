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
#include "LocalNodeNemesisHashTestUtils.h"
#include "LocalNodeTestUtils.h"
#include "catapult/crypto/OpensslKeyUtils.h"
#include "catapult/extensions/ProcessBootstrapper.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/local/server/LocalNode.h"
#include "tests/test/core/StorageTestUtils.h"
#include "tests/test/nemesis/NemesisCompatibleConfiguration.h"
#include "tests/test/nodeps/Filesystem.h"

namespace catapult { namespace test {

	/// Traits for a peer node.
	struct LocalNodePeerTraits {
		static constexpr auto CountersToLocalNodeStats = test::CountersToPeerLocalNodeStats;
		static constexpr auto AddPluginExtensions = test::AddPeerPluginExtensions;
	};

	/// Traits for an api node.
	struct LocalNodeApiTraits {
		static constexpr auto CountersToLocalNodeStats = test::CountersToBasicLocalNodeStats;
		static constexpr auto AddPluginExtensions = test::AddApiPluginExtensions;
	};

	/// Common test context for local node tests.
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
				const consumer<config::CatapultConfiguration&>& configTransform,
				const std::string& tempDirPostfix)
				: m_nodeFlag(nodeFlag)
				, m_nodes(nodes)
				, m_configTransform(configTransform)
				, m_serverKeyPair(loadServerKeyPair())
				, m_partnerServerKeyPair(LoadPartnerServerKeyPair())
				, m_tempDir("lntc" + tempDirPostfix)
				, m_partnerTempDir("lntc_partner" + tempDirPostfix) {
			initializeDataDirectory(m_tempDir.name());

			if (HasFlag(NodeFlag::With_Partner, nodeFlag)) {
				initializeDataDirectory(m_partnerTempDir.name());

				// need to call configTransform first so that partner node loads all required transaction plugins
				auto config = CreatePrototypicalCatapultConfiguration(m_partnerTempDir.name());
				m_configTransform(config);
				m_pLocalPartnerNode = BootLocalPartnerNode(std::move(config), m_partnerServerKeyPair, nodeFlag);
			}

			if (!HasFlag(NodeFlag::Require_Explicit_Boot, nodeFlag))
				boot();
		}

	private:
		void initializeDataDirectory(const std::string& directory) const {
			PrepareStorage(directory);
			PrepareConfiguration(directory, m_nodeFlag);

			if (HasFlag(NodeFlag::Verify_Receipts, m_nodeFlag))
				SetNemesisReceiptsHash(directory);

			if (HasFlag(NodeFlag::Verify_State, m_nodeFlag)) {
				auto config = CreatePrototypicalCatapultConfiguration(directory);
				prepareCatapultConfiguration(config);

				SetNemesisStateHash(directory, config);
			}
		}

	public:
		/// Gets the data directory.
		std::string dataDirectory() const {
			return m_tempDir.name();
		}

		/// Gets the resources directory.
		std::string resourcesDirectory() const {
			return m_tempDir.name() + "/resources";
		}

		/// Gets the public key of the (first) local node.
		const Key& publicKey() const {
			return m_serverKeyPair.publicKey();
		}

		/// Gets the primary (first) local node.
		local::LocalNode& localNode() const {
			return *m_pLocalNode;
		}

		/// Gets the node stats.
		auto stats() const {
			return TTraits::CountersToLocalNodeStats(m_pLocalNode->counters());
		}

		/// Loads saved height from persisted state.
		Height loadSavedStateChainHeight() const {
			auto path = boost::filesystem::path(m_tempDir.name()) / "state" / "supplemental.dat";
			io::RawFile file(path.generic_string(), io::OpenMode::Read_Only);
			return io::Read<Height>(file);
		}

	public:
		/// Creates a copy of the default catapult configuration.
		config::CatapultConfiguration createConfig() const {
			return CreatePrototypicalCatapultConfiguration(m_tempDir.name());
		}

		/// Prepares a fresh data \a directory and returns corresponding configuration.
		config::CatapultConfiguration prepareFreshDataDirectory(const std::string& directory) const {
			initializeDataDirectory(directory);

			auto config = CreatePrototypicalCatapultConfiguration(directory);
			prepareCatapultConfiguration(config);
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
		std::unique_ptr<local::LocalNode> boot(config::CatapultConfiguration&& config) {
			return boot(std::move(config), [](const auto&) {});
		}

		/// Boots a new local node allowing additional customization via \a configure.
		std::unique_ptr<local::LocalNode> boot(const consumer<extensions::ProcessBootstrapper&>& configure) {
			return boot(createConfig(), configure);
		}

		/// Resets this context and shuts down the local node.
		void reset() {
			if (m_pLocalPartnerNode)
				CATAPULT_THROW_INVALID_ARGUMENT("local node's partner node is expected to be uninitialized");

			m_pLocalNode->shutdown();
			m_pLocalNode.reset();
		}

	private:
		std::unique_ptr<local::LocalNode> boot(
				config::CatapultConfiguration&& config,
				const consumer<extensions::ProcessBootstrapper&>& configure) {
			prepareCatapultConfiguration(config);

			auto pBootstrapper = std::make_unique<extensions::ProcessBootstrapper>(
					std::move(config),
					resourcesDirectory(),
					extensions::ProcessDisposition::Production,
					"LocalNodeTests");
			pBootstrapper->addStaticNodes(m_nodes);

			auto& extensionManager = pBootstrapper->extensionManager();
			extensionManager.addServiceRegistrar(std::make_unique<CapturingServiceRegistrar>(m_capturedServiceState));
			pBootstrapper->loadExtensions();

			configure(*pBootstrapper);

			return local::CreateLocalNode(m_serverKeyPair, std::move(pBootstrapper));
		}

	private:
		void prepareCatapultConfiguration(config::CatapultConfiguration& config) const {
			PrepareCatapultConfiguration(config, TTraits::AddPluginExtensions, m_nodeFlag);
			m_configTransform(config);
		}

		crypto::KeyPair loadServerKeyPair() const {
			// can pass empty string to CreateCatapultConfiguration because this config is only being used to get boot key
			auto config = CreatePrototypicalCatapultConfiguration("");
			m_configTransform(config);
			return crypto::ReadKeyPairFromPrivateKeyPemFile(config::GetPrivateKeyPemFilename(config.User));
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
		consumer<config::CatapultConfiguration&> m_configTransform;
		crypto::KeyPair m_serverKeyPair;
		crypto::KeyPair m_partnerServerKeyPair;
		TempDirectoryGuard m_tempDir;
		TempDirectoryGuard m_partnerTempDir;

		std::unique_ptr<local::LocalNode> m_pLocalPartnerNode;
		std::unique_ptr<local::LocalNode> m_pLocalNode;
		mutable CapturedServiceState m_capturedServiceState;
	};
}}
