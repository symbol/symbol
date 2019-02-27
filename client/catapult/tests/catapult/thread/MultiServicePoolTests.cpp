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

#include "catapult/thread/MultiServicePool.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/TestHarness.h"
#include <mutex>

namespace catapult { namespace thread {

#define TEST_CLASS MultiServicePoolTests

	// region FooService

	namespace {
		class ShutdownIds {
		public:
			void push_back(size_t id) {
				std::lock_guard<std::mutex> lock(m_mutex);
				m_ids.push_back(id);
			}

			std::vector<size_t> toVector() {
				std::lock_guard<std::mutex> lock(m_mutex);
				return m_ids;
			}

		private:
			std::mutex m_mutex;
			std::vector<size_t> m_ids;
		};

		// the only requirement for a compatible service is to have a shutdown function
		class FooService {
		public:
			FooService(const std::string& threadPoolTag, size_t id, ShutdownIds& shutdownIds)
					: m_threadPoolTag(threadPoolTag)
					, m_id(id)
					, m_shutdownIds(shutdownIds)
			{}

		public:
			std::string threadPoolTag() const {
				return m_threadPoolTag;
			}

			size_t id() const {
				return m_id;
			}

		public:
			void shutdown() {
				// since clearing dependencies unblocks shutdown of pool, first push shutdown id into vector
				m_shutdownIds.push_back(m_id);
				m_dependencies.clear();
			}

			void addDependency(const std::shared_ptr<void>& pDependency) {
				m_dependencies.push_back(pDependency);
			}

		private:
			std::string m_threadPoolTag;
			size_t m_id;
			ShutdownIds& m_shutdownIds;
			std::vector<std::shared_ptr<void>> m_dependencies;
		};

		static std::shared_ptr<FooService> CreateFooService(
				const std::shared_ptr<IoThreadPool>& pPool,
				size_t id,
				ShutdownIds& shutdownIds) {
			return std::make_shared<FooService>(pPool->tag(), id, shutdownIds);
		}

		void AssertService(const FooService& service, const std::string& expectedThreadPoolTag, size_t expectedId) {
			// Assert:
			std::ostringstream message;
			message << "expected id - " << expectedId << ", expected tag - " << expectedThreadPoolTag;
			EXPECT_EQ(expectedThreadPoolTag, service.threadPoolTag()) << message.str();
			EXPECT_EQ(expectedId, service.id()) << message.str();
		}
	}

	// endregion

	// region create

	TEST(TEST_CLASS, CanCreatePoolWithSpecificNumberOfThreads) {
		// Act:
		MultiServicePool pool("foo", 3);

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(0u, pool.numServiceGroups());
		EXPECT_EQ(0u, pool.numServices());
	}

	TEST(TEST_CLASS, CanCreatePoolWithDefaultNumberOfThreads) {
		// Act:
		MultiServicePool pool("foo", MultiServicePool::DefaultPoolConcurrency());

		// Assert:
		EXPECT_EQ(std::thread::hardware_concurrency(), pool.numWorkerThreads());
		EXPECT_EQ(0u, pool.numServiceGroups());
		EXPECT_EQ(0u, pool.numServices());
	}

	// endregion

	// region pushServiceGroup

	TEST(TEST_CLASS, CanRegisterExternalService) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3);
		auto pExternalPool = utils::UniqueToShared(test::CreateStartedIoThreadPool(1));

		// Act:
		auto pService = pool.pushServiceGroup("beta")->registerService(CreateFooService(pExternalPool, 7u, shutdownIds));

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(1u, pool.numServiceGroups());
		EXPECT_EQ(1u, pool.numServices());

		AssertService(*pService, "IoThreadPool", 7);
	}

	TEST(TEST_CLASS, CanPushSingleService) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3);

		// Act:
		auto pService = pool.pushServiceGroup("beta")->pushService(CreateFooService, 7u, shutdownIds);

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(1u, pool.numServiceGroups());
		EXPECT_EQ(1u, pool.numServices());

		AssertService(*pService, "foo IoThreadPool", 7);
	}

	TEST(TEST_CLASS, CanPushSingleServiceGroupWithMultipleServices) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3);

		// Act:
		auto pGroup = pool.pushServiceGroup("beta");
		auto pService1 = pGroup->pushService(CreateFooService, 7u, shutdownIds);
		auto pService2 = pGroup->pushService(CreateFooService, 10u, shutdownIds);

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(1u, pool.numServiceGroups());
		EXPECT_EQ(2u, pool.numServices());
		EXPECT_EQ(2u, pGroup->numServices());

		AssertService(*pService1, "foo IoThreadPool", 7);
		AssertService(*pService2, "foo IoThreadPool", 10);
	}

	// endregion

	// region pushIsolatedPool

	namespace {
		template<typename TCreatePool>
		void AssertCanAddSingleIsolatedPool(size_t expectedNumWorkerThreads, TCreatePool createIsolatedPool) {
			// Arrange:
			ShutdownIds shutdownIds;
			MultiServicePool pool("foo", 3);

			// Act:
			auto pPool = createIsolatedPool(pool, "pool");

			// Assert:
			EXPECT_EQ(3u + expectedNumWorkerThreads, pool.numWorkerThreads());
			EXPECT_EQ(0u, pool.numServiceGroups());
			EXPECT_EQ(1u, pool.numServices());

			EXPECT_EQ(expectedNumWorkerThreads, pPool->numWorkerThreads());
			EXPECT_EQ("pool IoThreadPool", pPool->tag());
		}

		void AssertCanAddSingleIsolatedPoolWithExplicitThreads(size_t numWorkerThreads, size_t expectedNumWorkerThreads) {
			// Assert:
			AssertCanAddSingleIsolatedPool(expectedNumWorkerThreads, [numWorkerThreads](auto& pool, const auto& name) {
				return pool.pushIsolatedPool(name, numWorkerThreads);
			});
		}
	}

	TEST(TEST_CLASS, CanAddSingleIsolatedPoolWithCustomNumberOfThreads) {
		// Assert
		AssertCanAddSingleIsolatedPoolWithExplicitThreads(2, 2);
	}

	TEST(TEST_CLASS, CanAddSingleIsolatedPoolWithDefaultNumberOfThreads_Explicit) {
		// Assert
		AssertCanAddSingleIsolatedPoolWithExplicitThreads(MultiServicePool::DefaultPoolConcurrency(), std::thread::hardware_concurrency());
	}

	TEST(TEST_CLASS, CanAddSingleIsolatedPoolWithDefaultNumberOfThreads_Implicit) {
		// Assert:
		AssertCanAddSingleIsolatedPool(std::thread::hardware_concurrency(), [](auto& pool, const auto& name) {
			return pool.pushIsolatedPool(name);
		});
	}

	namespace {
		template<typename TCreatePool>
		void AssertCanAddSingleMergedPool(TCreatePool createIsolatedPool) {
			// Arrange:
			ShutdownIds shutdownIds;
			MultiServicePool pool("foo", 3, MultiServicePool::IsolatedPoolMode::Disabled);

			// Act:
			auto pPool = createIsolatedPool(pool, "pool");

			// Assert: no new threads were spawned
			EXPECT_EQ(3u, pool.numWorkerThreads());
			EXPECT_EQ(0u, pool.numServiceGroups());
			EXPECT_EQ(0u, pool.numServices());

			// - the returned pool is actually the main pool
			EXPECT_EQ(3u, pPool->numWorkerThreads());
			EXPECT_EQ("foo IoThreadPool", pPool->tag());
		}
	}

	TEST(TEST_CLASS, CanAddSingleMergedPoolWithCustomNumberOfThreads) {
		// Assert:
		AssertCanAddSingleMergedPool([](auto& pool, const auto& name) {
			return pool.pushIsolatedPool(name, 2);
		});
	}

	TEST(TEST_CLASS, CanAddSingleMergedPoolWithDefaultNumberOfThreads) {
		// Assert:
		AssertCanAddSingleMergedPool([](auto& pool, const auto& name) {
			return pool.pushIsolatedPool(name);
		});
	}

	// endregion

	// region pushServiceGroup / pushIsolatedPool

	TEST(TEST_CLASS, CanAddMultipleServices) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3);
		auto pExternalPool = utils::UniqueToShared(test::CreateStartedIoThreadPool(1));

		// Act:
		auto pGroup1 = pool.pushServiceGroup("alpha");
		auto pPool1 = pool.pushIsolatedPool("pool 1", 2 );
		auto pGroup2 = pool.pushServiceGroup("beta");
		auto pPool2 = pool.pushIsolatedPool("pool 2", 4);

		auto pService1 = pGroup1->pushService(CreateFooService, 7u, shutdownIds);
		auto pService2 = pGroup1->registerService(CreateFooService(pExternalPool, 9u, shutdownIds));
		auto pService3 = pGroup2->pushService(CreateFooService, 11u, shutdownIds);
		auto pService4 = pGroup2->pushService(CreateFooService, 77u, shutdownIds);
		auto pService5 = pGroup1->pushService(CreateFooService, 82u, shutdownIds);
		auto pService6 = pGroup2->pushService(CreateFooService, 22u, shutdownIds);
		auto pService7 = pGroup1->registerService(CreateFooService(pExternalPool, 99u, shutdownIds));

		// Assert:
		EXPECT_EQ(9u, pool.numWorkerThreads());
		EXPECT_EQ(2u, pPool1->numWorkerThreads());
		EXPECT_EQ(4u, pPool2->numWorkerThreads());

		EXPECT_EQ(2u, pool.numServiceGroups());
		EXPECT_EQ(9u, pool.numServices());
		EXPECT_EQ(4u, pGroup1->numServices());
		EXPECT_EQ(3u, pGroup2->numServices());

		AssertService(*pService1, "foo IoThreadPool", 7);
		AssertService(*pService2, "IoThreadPool", 9);
		AssertService(*pService3, "foo IoThreadPool", 11);
		AssertService(*pService4, "foo IoThreadPool", 77);
		AssertService(*pService5, "foo IoThreadPool", 82);
		AssertService(*pService6, "foo IoThreadPool", 22);
		AssertService(*pService7, "IoThreadPool", 99);
	}

	// endregion

	// region shutdown - basic

	namespace {
		void AssertPoolIsShutdown(const MultiServicePool& pool) {
			// Assert:
			EXPECT_EQ(0u, pool.numWorkerThreads());
			EXPECT_EQ(0u, pool.numServiceGroups());
			EXPECT_EQ(0u, pool.numServices());
		}

		void AssertCanShutdownPool(size_t count) {
			// Arrange:
			ShutdownIds shutdownIds;
			MultiServicePool pool("foo", 3);
			auto pExternalPool = utils::UniqueToShared(test::CreateStartedIoThreadPool(1));

			// - add services
			auto pGroup1 = pool.pushServiceGroup("alpha");
			pool.pushIsolatedPool("pool 1", 2);
			auto pGroup2 = pool.pushServiceGroup("beta");
			pool.pushIsolatedPool("pool 2", 4);

			pGroup1->pushService(CreateFooService, 7u, shutdownIds);
			pGroup1->registerService(CreateFooService(pExternalPool, 9u, shutdownIds));
			pGroup2->pushService(CreateFooService, 11u, shutdownIds);
			pGroup2->pushService(CreateFooService, 77u, shutdownIds);
			pGroup1->pushService(CreateFooService, 82u, shutdownIds);
			pGroup2->pushService(CreateFooService, 22u, shutdownIds);
			pGroup1->registerService(CreateFooService(pExternalPool, 99u, shutdownIds));

			// - release groups
			pGroup1.reset();
			pGroup2.reset();

			// Act:
			for (auto i = 0u; i < count; ++i)
				pool.shutdown();

			// Assert: the services have been shutdown and destroyed (in reverse order)
			AssertPoolIsShutdown(pool);

			// - note that all 'beta group' services are shutdown before 'pool 1' and all 'alpha group' services
			std::vector<size_t> expectedShutdownIds{ 22, 77, 11, 99, 82, 9, 7 };
			EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
		}
	}

	TEST(TEST_CLASS, CanShutdownPool) {
		// Assert:
		AssertCanShutdownPool(1);
	}

	TEST(TEST_CLASS, PoolShutdownIsIdempotent) {
		// Assert:
		AssertCanShutdownPool(7);
	}

	TEST(TEST_CLASS, PoolDestructorShutsDownPool) {
		// Arrange:
		ShutdownIds shutdownIds;
		{
			MultiServicePool pool("foo", 3);

			// Act:
			pool.pushServiceGroup("alpha")->pushService(CreateFooService, 7u, shutdownIds);
			pool.pushServiceGroup("beta")->pushService(CreateFooService, 11u, shutdownIds);
			pool.pushServiceGroup("gamma")->pushService(CreateFooService, 10u, shutdownIds);
		}

		// Assert: the services have been shutdown and destroyed (in reverse order)
		std::vector<size_t> expectedShutdownIds{ 10, 11, 7 };
		EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
	}

	// endregion

	// region shutdown - blocking

	namespace {
		template<typename TCreateService>
		void AssertShutdownWaitsForOutstandingServices(TCreateService createService) {
			// Arrange: create a pool with three services and extend the life of the second service
			ShutdownIds shutdownIds;
			MultiServicePool pool("foo", 3);
			pool.pushServiceGroup("alpha")->pushService(CreateFooService, 7u, shutdownIds);
			auto pService = createService(pool, 11u, shutdownIds);
			pool.pushServiceGroup("beta")->pushService(CreateFooService, 10u, shutdownIds);

			// Act: keep the second service alive on another thread
			std::thread([&shutdownIds, pService = std::move(pService)] {
				WAIT_FOR_VALUE_EXPR(2u, shutdownIds.toVector().size());
				// - wait to see if additional services shutdown erroneously
				test::Pause();

				// Assert: service has been shutdown but has not been destroyed yet
				std::vector<size_t> expectedShutdownIds{ 10, 11 };
				EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
				EXPECT_TRUE(!!pService);
			}).detach();

			// - shutdown and wait for all services to complete
			pool.shutdown();
			WAIT_FOR_ZERO_EXPR(pool.numWorkerThreads());

			// Assert: the services and pool are destroyed when the thread that called shutdown has
			//         the last outstanding service references
			AssertPoolIsShutdown(pool);

			std::vector<size_t> expectedShutdownIds{ 10, 11, 7 };
			EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
		}
	}

	TEST(TEST_CLASS, ShutdownWaitsForOutstandingServices) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			auto pServiceGroup = pool.pushServiceGroup("zeta");
			return pServiceGroup->pushService(CreateFooService, id, shutdownIds);
		});
	}

	TEST(TEST_CLASS, ShutdownWaitsForOutstandingExternalServices) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			auto pExternalPool = utils::UniqueToShared(test::CreateStartedIoThreadPool(1));

			auto pServiceGroup = pool.pushServiceGroup("zeta");
			return pServiceGroup->registerService(CreateFooService(pExternalPool, id, shutdownIds));
		});
	}

	TEST(TEST_CLASS, ShutdownWaitsForOutstandingServiceGroups) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			// - add an extra service because service groups do not have ids
			auto pServiceGroup = pool.pushServiceGroup("zeta");
			pool.pushServiceGroup("epsilon")->pushService(CreateFooService, id, shutdownIds);
			return pServiceGroup;
		});
	}

	TEST(TEST_CLASS, ShutdownWaitsForOutstandingIsolatedPools) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			// - add an extra service because isolated pools do not have ids
			auto pPool = pool.pushIsolatedPool("pool", 1);
			pool.pushServiceGroup("epsilon")->pushService(CreateFooService, id, shutdownIds);
			return pPool;
		});
	}

	TEST(TEST_CLASS, ShutdownWaitsForOutstandingPrimaryThreadPool) {
		// Arrange: create a pool with three services and a simulated subservice
		//          this test ensures correct shutdown when a service passes the primary pool to a subservice that can outlive it
		//          the multi service pool must wait for all references to its primary pool to be released before shutting down
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3);
		std::shared_ptr<IoThreadPool> pSimulatedSubService;
		pool.pushServiceGroup("alpha")->pushService(CreateFooService, 7u, shutdownIds);
		pool.pushServiceGroup("epsilon")->pushService([&pSimulatedSubService, &shutdownIds](const auto& pPool) {
			pSimulatedSubService = pPool;
			return CreateFooService(pPool, 11u, shutdownIds);
		});
		pool.pushServiceGroup("beta")->pushService(CreateFooService, 10u, shutdownIds);

		// Act: keep the subservice alive on another thread
		std::thread([&pool, &shutdownIds, pService = std::move(pSimulatedSubService)] {
			WAIT_FOR_ZERO_EXPR(pool.numServiceGroups());

			// Assert: all services have been shutdown but the subservice has not
			std::vector<size_t> expectedShutdownIds{ 10, 11, 7 };
			EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
			EXPECT_TRUE(!!pService);
		}).detach();

		// - shutdown and wait for all services to complete
		pool.shutdown();
		WAIT_FOR_ZERO_EXPR(pool.numWorkerThreads());

		// Assert: the services and pool are destroyed when the thread that called shutdown has
		//         the last outstanding service and primary pool references
		AssertPoolIsShutdown(pool);

		std::vector<size_t> expectedShutdownIds{ 10, 11, 7 };
		EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
	}

	TEST(TEST_CLASS, ShutdownShutsDownAllServicesBeforeMergedSubPool) {
		// Arrange: create a pool with two services and a sub-pool and extend the life of the sub-pool
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3, MultiServicePool::IsolatedPoolMode::Disabled);
		pool.pushServiceGroup("alpha")->pushService(CreateFooService, 7u, shutdownIds);
		auto pSubPool = pool.pushIsolatedPool("pool", 1);
		pool.pushServiceGroup("beta")->pushService(CreateFooService, 10u, shutdownIds);

		// Act: keep the "isolated" pool alive on another thread
		std::thread([&shutdownIds, pSubPool = std::move(pSubPool)] {
			WAIT_FOR_VALUE_EXPR(2u, shutdownIds.toVector().size());
			// - wait to see if additional services shutdown erroneously
			test::Pause();

			// Assert: all services have been shutdown and destroyed
			//         in merged mode, there is only a single pool, so isolated pools have no effect on shutdown (and cannot block it)
			std::vector<size_t> expectedShutdownIds{ 10, 7 };
			EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
			EXPECT_TRUE(!!pSubPool);
		}).detach();

		// - shutdown and wait for all services to complete
		pool.shutdown();
		WAIT_FOR_ZERO_EXPR(pool.numWorkerThreads());

		// Assert: the services and pool are destroyed when the thread that called shutdown has
		//         the last outstanding service references
		AssertPoolIsShutdown(pool);

		std::vector<size_t> expectedShutdownIds{ 10, 7 };
		EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
	}

	// endregion

	// region shutdown - interdependencies

	TEST(TEST_CLASS, ShutdownDoesNotAllowInterdependentServicesAcrossServiceGroups) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3);

		// - register dependent services (across service groups)
		auto pService1 = pool.pushServiceGroup("alpha")->pushService(CreateFooService, 11u, shutdownIds);
		auto pService2 = pool.pushServiceGroup("beta")->pushService(CreateFooService, 12u, shutdownIds);
		pService1->addDependency(pService2);
		pService2->addDependency(pService1);

		std::thread([&shutdownIds, pService1 = pService1.get()] {
			WAIT_FOR_ONE_EXPR(shutdownIds.toVector().size());
			// - wait to see if additional services shutdown erroneously
			test::Pause();

			// Assert: shutdown is blocked shutting down first service
			std::vector<size_t> expectedShutdownIds{ 12 };
			EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());

			// Act: force shutdown of first service
			pService1->shutdown();
		}).detach();

		// Act: shutdown and wait for all services to complete
		pService2.reset();
		pService1.reset();
		pool.shutdown();

		// Assert:
		AssertPoolIsShutdown(pool);

		// - note that first service shutdown is called twice (once explicitly in the other thread)
		std::vector<size_t> expectedShutdownIds{ 12, 11, 11 };
		EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
	}

	TEST(TEST_CLASS, ShutdownAllowsInterdependentServicesWithinSameServiceGroup) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool("foo", 3);
		auto pGroup = pool.pushServiceGroup("alpha");

		// - register dependent services (within a single service groups)
		auto pService1 = pGroup->pushService(CreateFooService, 11u, shutdownIds);
		auto pService2 = pGroup->pushService(CreateFooService, 12u, shutdownIds);
		pService1->addDependency(pService2);
		pService2->addDependency(pService1);

		// Act: shutdown and wait for all services to complete
		pService2.reset();
		pService1.reset();
		pGroup.reset();
		pool.shutdown();

		// Assert:
		AssertPoolIsShutdown(pool);

		std::vector<size_t> expectedShutdownIds{ 12, 11 };
		EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
	}

	// endregion
}}
