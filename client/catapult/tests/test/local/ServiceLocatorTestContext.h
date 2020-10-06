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
#include "LocalTestUtils.h"
#include "catapult/cache_tx/MemoryUtCache.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/extensions/LocalNodeChainScore.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/utils/NetworkTime.h"
#include "tests/test/core/SchedulerTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/other/mocks/MockFinalizationSubscriber.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"
#include "tests/test/other/mocks/MockTransactionStatusSubscriber.h"

namespace catapult { namespace test {

	/// Wrapper around ServiceState.
	class ServiceTestState {
	public:
		/// Creates the test state.
		ServiceTestState() : ServiceTestState(cache::CatapultCache({}))
		{}

		/// Creates the test state around \a cache.
		explicit ServiceTestState(cache::CatapultCache&& cache) : ServiceTestState(std::move(cache), CreateDefaultNetworkTimeSupplier())
		{}

		/// Creates the test state around \a cache and \a timeSupplier.
		ServiceTestState(cache::CatapultCache&& cache, const supplier<Timestamp>& timeSupplier)
				: m_config(CreatePrototypicalCatapultConfiguration())
				, m_nodes(
						std::numeric_limits<size_t>::max(),
						model::NodeIdentityEqualityStrategy::Key_And_Host,
						GetBanSettings(m_config),
						timeSupplier,
						[](auto) { return true; })
				, m_catapultCache(std::move(cache))
				, m_storage(std::make_unique<mocks::MockMemoryBlockStorage>(), std::make_unique<mocks::MockMemoryBlockStorage>())
				, m_pUtCache(CreateUtCacheProxy())
				, m_nodeSubscriber(m_nodes)
				, m_pluginManager(m_config.BlockChain, plugins::StorageConfiguration(), m_config.User, m_config.Inflation)
				, m_pool("service locator test context", 2)
				, m_state(
						m_config,
						m_nodes,
						m_catapultCache,
						m_storage,
						m_score,
						*m_pUtCache,
						timeSupplier,
						m_finalizationSubscriber,
						m_nodeSubscriber,
						m_stateChangeSubscriber,
						m_transactionStatusSubscriber,
						m_counters,
						m_pluginManager,
						m_pool)
		{}

	public:
		/// Gets the service state.
		auto& state() {
			return m_state;
		}

	public:
		/// Gets the config.
		auto& config() const {
			return m_config;
		}

		/// Gets the finalization subscriber.
		const auto& finalizationSubscriber() const {
			return m_finalizationSubscriber;
		}

		/// Gets the node subscriber.
		const auto& nodeSubscriber() const {
			return m_nodeSubscriber;
		}

		/// Gets the node subscriber.
		auto& nodeSubscriber() {
			return m_nodeSubscriber;
		}

		/// Gets the state change subscriber.
		const auto& stateChangeSubscriber() const {
			return m_stateChangeSubscriber;
		}

		/// Gets the transaction status subscriber.
		const auto& transactionStatusSubscriber() const {
			return m_transactionStatusSubscriber;
		}

		/// Gets the counters.
		auto& counters() {
			return m_counters;
		}

		/// Gets the plugin manager.
		auto& pluginManager() {
			return m_pluginManager;
		}

	private:
		static ionet::BanSettings GetBanSettings(const config::CatapultConfiguration& config) {
			const auto& banConfig = config.Node.Banning;
			ionet::BanSettings banSettings;
			banSettings.DefaultBanDuration = banConfig.DefaultBanDuration;
			banSettings.MaxBanDuration = banConfig.MaxBanDuration;
			banSettings.KeepAliveDuration = banConfig.KeepAliveDuration;
			banSettings.MaxBannedNodes = banConfig.MaxBannedNodes;
			return banSettings;
		}

	private:
		config::CatapultConfiguration m_config;
		ionet::NodeContainer m_nodes;
		cache::CatapultCache m_catapultCache;
		io::BlockStorageCache m_storage;
		extensions::LocalNodeChainScore m_score;
		std::unique_ptr<cache::MemoryUtCacheProxy> m_pUtCache;

		mocks::MockFinalizationSubscriber m_finalizationSubscriber;
		mocks::MockNodeSubscriber m_nodeSubscriber;
		mocks::MockStateChangeSubscriber m_stateChangeSubscriber;
		mocks::MockTransactionStatusSubscriber m_transactionStatusSubscriber;

		std::vector<utils::DiagnosticCounter> m_counters;
		plugins::PluginManager m_pluginManager;
		thread::MultiServicePool m_pool;

		extensions::ServiceState m_state;
	};

	/// Test context for extension service tests.
	template<typename TTraits>
	class ServiceLocatorTestContext {
	public:
		/// Creates the test context.
		ServiceLocatorTestContext()
				: m_keys(test::GenerateRandomByteArray<Key>(), GenerateKeyPair())
				, m_pLocator(std::make_unique<extensions::ServiceLocator>(m_keys))
		{}

		/// Creates the test context around \a cache.
		explicit ServiceLocatorTestContext(cache::CatapultCache&& cache)
				: ServiceLocatorTestContext(std::move(cache), CreateDefaultNetworkTimeSupplier())
		{}

		/// Creates the test context around \a cache and \a timeSupplier.
		ServiceLocatorTestContext(cache::CatapultCache&& cache, const supplier<Timestamp>& timeSupplier)
				: m_keys(test::GenerateRandomByteArray<Key>(), GenerateKeyPair())
				, m_pLocator(std::make_unique<extensions::ServiceLocator>(m_keys))
				, m_testState(std::move(cache), timeSupplier)
		{}

	public:
		/// Gets the value of the counter named \a counterName.
		uint64_t counter(const std::string& counterName) const {
			for (const auto& counter : m_pLocator->counters()) {
				if (HasName(counter, counterName))
					return counter.value();
			}

			CATAPULT_THROW_INVALID_ARGUMENT_1("could not find counter with name", counterName);
		}

	public:
		/// Gets the public key.
		const auto& publicKey() const {
			return m_keys.caPublicKey();
		}

		/// Gets the service locator.
		auto& locator() {
			return *m_pLocator;
		}

		/// Gets the service locator.
		const auto& locator() const {
			return *m_pLocator;
		}

		/// Gets the test state.
		auto& testState() {
			return m_testState;
		}

		/// Gets the test state.
		const auto& testState() const {
			return m_testState;
		}

	public:
		/// Boots the service around \a args.
		template<typename... TArgs>
		void boot(TArgs&&... args) {
			auto pRegistrar = TTraits::CreateRegistrar(std::forward<TArgs>(args)...);
			pRegistrar->registerServiceCounters(*m_pLocator);
			pRegistrar->registerServices(*m_pLocator, m_testState.state());
		}

		/// Shuts down the service.
		void shutdown() {
			m_testState.state().pool().shutdown();
		}

	protected:
		/// Destroys the underlying service locator.
		void destroy() {
			shutdown();
			m_pLocator.reset();
		}

	private:
		static bool HasName(const utils::DiagnosticCounter& counter, const std::string& name) {
			return name == counter.id().name();
		}

	private:
		config::CatapultKeys m_keys;
		std::unique_ptr<extensions::ServiceLocator> m_pLocator;

		ServiceTestState m_testState;
	};

	/// Extracts a task named \a taskName from \a context, which is expected to contain \a numExpectedTasks tasks,
	/// and forwards it to \a action.
	/// \note \a context is expected to be booted.
	template<typename TTestContext, typename TAction>
	void RunTaskTestPostBoot(TTestContext& context, size_t numExpectedTasks, const std::string& taskName, TAction action) {
		// Sanity: expected number of tasks should be registered
		const auto& tasks = context.testState().state().tasks();
		EXPECT_EQ(numExpectedTasks, tasks.size());

		// Act: find the desired task
		auto iter = std::find_if(tasks.cbegin(), tasks.cend(), [&taskName](const auto& task) {
			return taskName == task.Name;
		});

		if (tasks.cend() == iter)
			CATAPULT_THROW_RUNTIME_ERROR_1("unable to find task with name", taskName);

		action(*iter);
	}

	/// Extracts a task named \a taskName from \a context, which is expected to contain \a numExpectedTasks tasks,
	/// and forwards it to \a action.
	template<typename TTestContext, typename TAction>
	void RunTaskTest(TTestContext& context, size_t numExpectedTasks, const std::string& taskName, TAction action) {
		// Arrange:
		context.boot();

		// Act:
		RunTaskTestPostBoot(context, numExpectedTasks, taskName, std::move(action));
	}

	/// Asserts that the named tasks (\a expectedTaskNames) exactly match the tasks registered in \a context.
	/// \note \a context is expected to be booted.
	template<typename TTestContext>
	void AssertRegisteredTasksPostBoot(TTestContext&& context, const std::unordered_set<std::string>& expectedTaskNames) {
		// Sanity: expected number of tasks should be registered
		const auto& tasks = context.testState().state().tasks();
		EXPECT_EQ(expectedTaskNames.size(), tasks.size());

		// Act:
		std::unordered_set<std::string> actualTaskNames;
		for (const auto& task : tasks) {
			actualTaskNames.insert(task.Name);

			// Sanity: check non-name properties
			AssertUnscheduledTask(task, task.Name);
		}

		// Assert:
		EXPECT_EQ(expectedTaskNames, actualTaskNames);
	}

	/// Asserts that the named tasks (\a expectedTaskNames) exactly match the tasks registered in \a context.
	template<typename TTestContext>
	void AssertRegisteredTasks(TTestContext&& context, const std::unordered_set<std::string>& expectedTaskNames) {
		// Arrange:
		context.boot();

		// Act + Assert:
		AssertRegisteredTasksPostBoot(context, expectedTaskNames);
	}
}}
