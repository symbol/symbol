#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/TestHarness.h"
#include <mutex>

namespace catapult { namespace thread {

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
				m_dependencies.clear();
				m_shutdownIds.push_back(m_id);
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
				const std::shared_ptr<IoServiceThreadPool>& pPool,
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

	TEST(MultiServicePoolTests, CanCreatePoolWithSpecificNumberOfThreads) {
		// Act:
		MultiServicePool pool(3, "foo");

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(0u, pool.numServiceGroups());
		EXPECT_EQ(0u, pool.numServices());
	}

	TEST(MultiServicePoolTests, CanCreatePoolWithDefaultNumberOfThreads) {
		// Act:
		MultiServicePool pool(MultiServicePool::DefaultPoolConcurrency(), "foo");

		// Assert:
		EXPECT_EQ(std::thread::hardware_concurrency(), pool.numWorkerThreads());
		EXPECT_EQ(0u, pool.numServiceGroups());
		EXPECT_EQ(0u, pool.numServices());
	}

	// endregion

	// region pushServiceGroup / pushIsolatedPool

	// region pushServiceGroup

	TEST(MultiServicePoolTests, CanRegisterExternalService) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool(3, "foo");
		auto pExternalPool = std::shared_ptr<IoServiceThreadPool>(test::CreateStartedIoServiceThreadPool(1));

		// Act:
		auto pService = pool.pushServiceGroup("beta")->registerService(CreateFooService(pExternalPool, 7u, shutdownIds));

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(1u, pool.numServiceGroups());
		EXPECT_EQ(1u, pool.numServices());

		AssertService(*pService, "IoServiceThreadPool", 7);
	}

	TEST(MultiServicePoolTests, CanPushSingleService) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool(3, "foo");

		// Act:
		auto pService = pool.pushServiceGroup("beta")->pushService(CreateFooService, 7u, shutdownIds);

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(1u, pool.numServiceGroups());
		EXPECT_EQ(1u, pool.numServices());

		AssertService(*pService, "foo IoServiceThreadPool", 7);
	}

	TEST(MultiServicePoolTests, CanPushSingleServiceGroupWithMultipleServices) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool(3, "foo");

		// Act:
		auto pGroup = pool.pushServiceGroup("beta");
		auto pService1 = pGroup->pushService(CreateFooService, 7u, shutdownIds);
		auto pService2 = pGroup->pushService(CreateFooService, 10u, shutdownIds);

		// Assert:
		EXPECT_EQ(3u, pool.numWorkerThreads());
		EXPECT_EQ(1u, pool.numServiceGroups());
		EXPECT_EQ(2u, pool.numServices());
		EXPECT_EQ(2u, pGroup->numServices());

		AssertService(*pService1, "foo IoServiceThreadPool", 7);
		AssertService(*pService2, "foo IoServiceThreadPool", 10);
	}

	// endregion

	// region pushIsolatedPool

	namespace {
		void AssertCanAddSingleIsolatedPool(size_t numWorkerThreads, size_t expectedNumWorkerThreads) {
			// Arrange:
			ShutdownIds shutdownIds;
			MultiServicePool pool(3, "foo");

			// Act:
			auto pPool = pool.pushIsolatedPool(numWorkerThreads, "pool");

			// Assert:
			EXPECT_EQ(3u + expectedNumWorkerThreads, pool.numWorkerThreads());
			EXPECT_EQ(0u, pool.numServiceGroups());
			EXPECT_EQ(1u, pool.numServices());

			EXPECT_EQ(expectedNumWorkerThreads, pPool->numWorkerThreads());
			EXPECT_EQ("pool IoServiceThreadPool", pPool->tag());
		}
	}

	TEST(MultiServicePoolTests, CanAddSingleIsolatedPoolWithCustomNumberOfThreads) {
		// Assert
		AssertCanAddSingleIsolatedPool(2, 2);
	}

	TEST(MultiServicePoolTests, CanAddSingleIsolatedPoolWithDefaultNumberOfThreads) {
		// Assert
		AssertCanAddSingleIsolatedPool(MultiServicePool::DefaultPoolConcurrency(), std::thread::hardware_concurrency());
	}

	// endregion

	TEST(MultiServicePoolTests, CanAddMultipleServices) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool(3, "foo");
		auto pExternalPool = std::shared_ptr<IoServiceThreadPool>(test::CreateStartedIoServiceThreadPool(1));

		// Act:
		auto pGroup1 = pool.pushServiceGroup("alpha");
		auto pPool1 = pool.pushIsolatedPool(2, "pool 1");
		auto pGroup2 = pool.pushServiceGroup("beta");
		auto pPool2 = pool.pushIsolatedPool(4, "pool 2");

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

		AssertService(*pService1, "foo IoServiceThreadPool", 7);
		AssertService(*pService2, "IoServiceThreadPool", 9);
		AssertService(*pService3, "foo IoServiceThreadPool", 11);
		AssertService(*pService4, "foo IoServiceThreadPool", 77);
		AssertService(*pService5, "foo IoServiceThreadPool", 82);
		AssertService(*pService6, "foo IoServiceThreadPool", 22);
		AssertService(*pService7, "IoServiceThreadPool", 99);
	}

	// endregion

	// region shutdown

	// region basic

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
			MultiServicePool pool(3, "foo");
			auto pExternalPool = std::shared_ptr<IoServiceThreadPool>(test::CreateStartedIoServiceThreadPool(1));

			// - add services
			auto pGroup1 = pool.pushServiceGroup("alpha");
			pool.pushIsolatedPool(2, "pool 1");
			auto pGroup2 = pool.pushServiceGroup("beta");
			pool.pushIsolatedPool(4, "pool 2");

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

	TEST(MultiServicePoolTests, CanShutdownPool) {
		// Assert:
		AssertCanShutdownPool(1);
	}

	TEST(MultiServicePoolTests, PoolShutdownIsIdempotent) {
		// Assert:
		AssertCanShutdownPool(7);
	}

	TEST(MultiServicePoolTests, PoolDestructorShutdownsPool) {
		// Arrange:
		ShutdownIds shutdownIds;
		{
			MultiServicePool pool(3, "foo");

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

	// region blocking

	namespace {
		template<typename TCreateService>
		void AssertShutdownWaitsForOutstandingServices(TCreateService createService) {
			// Arrange: create a pool with three services and extend the life of the second service
			ShutdownIds shutdownIds;
			MultiServicePool pool(3, "foo");
			pool.pushServiceGroup("alpha")->pushService(CreateFooService, 7u, shutdownIds);
			auto pService = createService(pool, 11u, shutdownIds);
			pool.pushServiceGroup("beta")->pushService(CreateFooService, 10u, shutdownIds);

			// Act: keep the second service alive on another thread
			std::thread([&shutdownIds, pService = std::move(pService)] {
				WAIT_FOR_VALUE_EXPR(shutdownIds.toVector().size(), 2u);
				// - wait to see if additional services shutdown erroneously
				test::Pause();

				// Assert: service has been shutdown but has not been destroyed yet
				std::vector<size_t> expectedShutdownIds{ 10, 11 };
				EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
				EXPECT_TRUE(!!pService);
			}).detach();

			// - shutdown and wait for all services to complete
			pool.shutdown();
			WAIT_FOR_VALUE_EXPR(pool.numWorkerThreads(), 0u);

			// Assert: the services and pool are destroyed when the thread that called shutdown has
			//         the last outstanding service references
			AssertPoolIsShutdown(pool);

			std::vector<size_t> expectedShutdownIds{ 10, 11, 7 };
			EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
		}
	}

	TEST(MultiServicePoolTests, ShutdownWaitsForOutstandingServices) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			auto pServiceGroup = pool.pushServiceGroup("zeta");
			return pServiceGroup->pushService(CreateFooService, id, shutdownIds);
		});
	}

	TEST(MultiServicePoolTests, ShutdownWaitsForOutstandingExternalServices) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			auto pExternalPool = std::shared_ptr<IoServiceThreadPool>(test::CreateStartedIoServiceThreadPool(1));

			auto pServiceGroup = pool.pushServiceGroup("zeta");
			return pServiceGroup->registerService(CreateFooService(pExternalPool, id, shutdownIds));
		});
	}

	TEST(MultiServicePoolTests, ShutdownWaitsForOutstandingServiceGroups) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			// - add an extra service because service groups do not have ids
			auto pServiceGroup = pool.pushServiceGroup("zeta");
			pool.pushServiceGroup("epsilon")->pushService(CreateFooService, id, shutdownIds);
			return pServiceGroup;
		});
	}

	TEST(MultiServicePoolTests, ShutdownWaitsForOutstandingIsolatedPools) {
		// Assert:
		AssertShutdownWaitsForOutstandingServices([](auto& pool, auto id, auto& shutdownIds) {
			// - add an extra service because isolated pools do not have ids
			auto pPool = pool.pushIsolatedPool(1, "pool");
			pool.pushServiceGroup("epsilon")->pushService(CreateFooService, id, shutdownIds);
			return pPool;
		});
	}

	TEST(MultiServicePoolTests, ShutdownWaitsForOutstandingPrimaryThreadPool) {
		// Arrange: create a pool with three services and a simulated subservice
		//          this test ensures correct shutdown when a service passes the primary pool to a subservice that can outlive it
		//          the multi service pool must wait for all references to its primary pool to be released before shutting down
		ShutdownIds shutdownIds;
		MultiServicePool pool(3, "foo");
		std::shared_ptr<IoServiceThreadPool> pSimulatedSubService;
		pool.pushServiceGroup("alpha")->pushService(CreateFooService, 7u, shutdownIds);
		pool.pushServiceGroup("epsilon")->pushService(
				[&pSimulatedSubService, &shutdownIds](const auto& pPool) {
					pSimulatedSubService = pPool;
					return CreateFooService(pPool, 11u, shutdownIds);
				});
		pool.pushServiceGroup("beta")->pushService(CreateFooService, 10u, shutdownIds);

		// Act: keep the subservice alive on another thread
		std::thread([&pool, &shutdownIds, pService = std::move(pSimulatedSubService)] {
			WAIT_FOR_VALUE_EXPR(pool.numServiceGroups(), 0u);

			// Assert: all services have been shutdown but the subservice has not
			std::vector<size_t> expectedShutdownIds{ 10, 11, 7 };
			EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
			EXPECT_TRUE(!!pService);
		}).detach();

		// - shutdown and wait for all services to complete
		pool.shutdown();
		WAIT_FOR_VALUE_EXPR(pool.numWorkerThreads(), 0u);

		// Assert: the services and pool are destroyed when the thread that called shutdown has
		//         the last outstanding service and primary pool references
		AssertPoolIsShutdown(pool);

		std::vector<size_t> expectedShutdownIds{ 10, 11, 7 };
		EXPECT_EQ(expectedShutdownIds, shutdownIds.toVector());
	}

	// endregion

	// region interdependencies

	TEST(MultiServicePoolTests, ShutdownDoesNotAllowInterdependentServicesAcrossServiceGroups) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool(3, "foo");

		// - register dependent services (across service groups)
		auto pService1 = pool.pushServiceGroup("alpha")->pushService(CreateFooService, 11u, shutdownIds);
		auto pService2 = pool.pushServiceGroup("beta")->pushService(CreateFooService, 12u, shutdownIds);
		pService1->addDependency(pService2);
		pService2->addDependency(pService1);

		std::thread([&shutdownIds, pService1 = pService1.get()] {
			WAIT_FOR_VALUE_EXPR(shutdownIds.toVector().size(), 1u);
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

	TEST(MultiServicePoolTests, ShutdownAllowsInterdependentServicesWithinSameServiceGroup) {
		// Arrange:
		ShutdownIds shutdownIds;
		MultiServicePool pool(3, "foo");
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

	// endregion
}}
