#include "catapult/extensions/ExtensionManager.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/thread/MultiServicePool.h"
#include "catapult/utils/NetworkTime.h"
#include "tests/test/local/LocalNodeTestState.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/other/ConsumerHandlerTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ExtensionManagerTests

	// region system plugins

	namespace {
		std::vector<std::string> GetDefaultSystemPluginNames() {
			return { "catapult.coresystem", "catapult.plugins.signature" };
		}
	}

	TEST(TEST_CLASS, CanGetSystemPluginNamesWithoutRegisteringCustomPlugins) {
		// Arrange:
		ExtensionManager manager;

		// Act:
		const auto& names = manager.systemPluginNames();

		// Assert:
		EXPECT_EQ(GetDefaultSystemPluginNames(), names);
	}

	TEST(TEST_CLASS, CanGetSystemPluginNamesAfterRegisteringCustomPlugins) {
		// Arrange:
		ExtensionManager manager;

		// Act:
		manager.registerSystemPlugin("foo");
		manager.registerSystemPlugin("bar");
		const auto& names = manager.systemPluginNames();

		// Assert:
		auto expectedNames = GetDefaultSystemPluginNames();
		expectedNames.push_back("foo");
		expectedNames.push_back("bar");
		EXPECT_EQ(expectedNames, names);
	}

	// endregion

	// region lifetime handlers

	struct PreLoadHandlerTraits {
		static auto CreateConsumer(const ExtensionManager& manager) {
			return manager.preLoadHandler();
		}

		static void AddConsumer(ExtensionManager& manager, const ExtensionManager::CacheConsumer& handler) {
			manager.addPreLoadHandler(handler);
		}

		static auto CreateConsumerData() {
			return cache::CatapultCache({});
		}
	};

	DEFINE_CONSUMER_HANDLER_TESTS(TEST_CLASS, ExtensionManager, PreLoadHandler);

	// endregion

	// region network time supplier

	TEST(TEST_CLASS, CanUseDefaultNetworkTimeSupplierWhenUnset) {
		// Arrange:
		ExtensionManager manager;
		auto supplier = manager.networkTimeSupplier();

		// Act: get the time
		auto startTime = utils::NetworkTime();
		auto time = supplier();
		auto endTime = utils::NetworkTime();

		// Assert: network time provider was used
		EXPECT_LE(startTime, time);
		EXPECT_LE(time, endTime);
	}

	TEST(TEST_CLASS, CanUseCustomNetworkTimeSupplier) {
		// Arrange:
		ExtensionManager manager;
		manager.setNetworkTimeSupplier([]() { return Timestamp(123); });
		auto supplier = manager.networkTimeSupplier();

		// Act:
		auto time = supplier();

		// Assert:
		EXPECT_EQ(Timestamp(123), time);
	}

	TEST(TEST_CLASS, CannotSetNetworkTimeSupplierMultipleTimes) {
		// Arrange:
		ExtensionManager manager;
		manager.setNetworkTimeSupplier([]() { return Timestamp(123); });

		// Act + Assert:
		EXPECT_THROW(manager.setNetworkTimeSupplier([]() { return Timestamp(123); }), catapult_invalid_argument);
	}

	// endregion

	// region block chain storage

	TEST(TEST_CLASS, CannotCreateBlockChainStorageWithoutRegistration) {
		// Arrange:
		ExtensionManager manager;

		// Act + Assert:
		EXPECT_THROW(manager.createBlockChainStorage(), catapult_runtime_error);
	}

	namespace {
		class UnsupportedBlockChainStorage : public BlockChainStorage {
		public:
			void loadFromStorage(const LocalNodeStateRef&, const plugins::PluginManager&) override {
				CATAPULT_THROW_RUNTIME_ERROR("loadFromStorage - not supported in mock");
			}

			void saveToStorage(const LocalNodeStateConstRef&) override {
				CATAPULT_THROW_RUNTIME_ERROR("saveToStorage - not supported in mock");
			}
		};
	}

	TEST(TEST_CLASS, CanCreateBlockChainStorageWithRegistration) {
		// Arrange:
		ExtensionManager manager;
		auto pRegisteredStorage = std::make_unique<UnsupportedBlockChainStorage>();
		const auto& registeredStorage = *pRegisteredStorage;

		// Act:
		manager.setBlockChainStorage(std::move(pRegisteredStorage));
		auto pStorage = manager.createBlockChainStorage();

		// Assert: saveToStorage should trigger storage exception
		ASSERT_TRUE(!!pStorage);
		EXPECT_EQ(&registeredStorage, pStorage.get());
		EXPECT_THROW(pStorage->saveToStorage(test::LocalNodeTestState().cref()), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotSetBlockChainStorageMultipleTimes) {
		// Arrange:
		ExtensionManager manager;
		manager.setBlockChainStorage(std::make_unique<UnsupportedBlockChainStorage>());

		// Act + Assert:
		EXPECT_THROW(manager.setBlockChainStorage(std::make_unique<UnsupportedBlockChainStorage>()), catapult_invalid_argument);
	}

	// endregion

	// region services

	namespace {
		struct ServiceRegistrarBreadcrumb {
			size_t Id;
			const ServiceLocator* pLocator;
			const ServiceState* pState;
		};

		class MockServiceRegistrar : public ServiceRegistrar {
		public:
			MockServiceRegistrar(size_t id, std::vector<ServiceRegistrarBreadcrumb>& breadcrumbs)
					: m_id(id)
					, m_breadcrumbs(breadcrumbs)
			{}

		public:
			ServiceRegistrarInfo info() const override {
				return { "MockServiceRegistrar-" + std::to_string(m_id), static_cast<ServiceRegistrarPhase>(m_id) };
			}

			void registerServiceCounters(ServiceLocator& locator) override {
				m_breadcrumbs.emplace_back(ServiceRegistrarBreadcrumb{ m_id, &locator, nullptr });
			}

			void registerServices(ServiceLocator& locator, ServiceState& state) override {
				m_breadcrumbs.emplace_back(ServiceRegistrarBreadcrumb{ m_id, &locator, &state });
			}

		private:
			size_t m_id;
			std::vector<ServiceRegistrarBreadcrumb>& m_breadcrumbs;
		};

		void AssertEqual(const ServiceRegistrarBreadcrumb& expected, const ServiceRegistrarBreadcrumb& actual) {
			auto message = "expected id " + std::to_string(expected.Id);
			EXPECT_EQ(expected.Id, actual.Id) << message;
			EXPECT_EQ(expected.pLocator, actual.pLocator) << message;
			EXPECT_EQ(expected.pState, actual.pState) << message;
		}
	}

	TEST(TEST_CLASS, CanRegisterZeroServices) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		ServiceLocator locator(keyPair);
		test::ServiceTestState testState;
		auto& state = testState.state();

		ExtensionManager manager;

		// Act + Assert: call does not throw
		manager.registerServices(locator, state);
	}

	TEST(TEST_CLASS, CanRegisterSingleService) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		ServiceLocator locator(keyPair);
		test::ServiceTestState testState;
		auto& state = testState.state();
		std::vector<ServiceRegistrarBreadcrumb> breadcrumbs;

		ExtensionManager manager;

		// Act:
		manager.addServiceRegistrar(std::make_unique<MockServiceRegistrar>(3, breadcrumbs));
		manager.registerServices(locator, state);

		// Assert:
		ASSERT_EQ(2u, breadcrumbs.size());
		AssertEqual(breadcrumbs[0], { 3u, &locator, nullptr });
		AssertEqual(breadcrumbs[1], { 3u, &locator, &state });
	}

	TEST(TEST_CLASS, CanRegisterMultipleServices) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		ServiceLocator locator(keyPair);
		test::ServiceTestState testState;
		auto& state = testState.state();
		std::vector<ServiceRegistrarBreadcrumb> breadcrumbs;

		ExtensionManager manager;

		// Act:
		manager.addServiceRegistrar(std::make_unique<MockServiceRegistrar>(3, breadcrumbs));
		manager.addServiceRegistrar(std::make_unique<MockServiceRegistrar>(6, breadcrumbs));
		manager.addServiceRegistrar(std::make_unique<MockServiceRegistrar>(5, breadcrumbs));
		manager.registerServices(locator, state);

		// Assert: notice that all counters are registered first
		ASSERT_EQ(6u, breadcrumbs.size());

		// - services are registered in order of phase
		auto i = 0u;
		for (auto id : { 3u, 5u, 6u }) {
			AssertEqual(breadcrumbs[i], { id, &locator, nullptr });
			AssertEqual(breadcrumbs[3 + i], { id, &locator, &state });
			++i;
		}
	}

	TEST(TEST_CLASS, RegisterServicesDestroysRegistrars) {
		// Arrange:
		auto keyPair = test::GenerateKeyPair();
		ServiceLocator locator(keyPair);
		test::ServiceTestState testState;
		auto& state = testState.state();
		std::vector<ServiceRegistrarBreadcrumb> breadcrumbs;

		ExtensionManager manager;

		// Act: call registerServices twice
		manager.addServiceRegistrar(std::make_unique<MockServiceRegistrar>(3, breadcrumbs));
		manager.registerServices(locator, state);
		manager.registerServices(locator, state);

		// Assert: the service was only registered once
		ASSERT_EQ(2u, breadcrumbs.size());
		AssertEqual(breadcrumbs[0], { 3u, &locator, nullptr });
		AssertEqual(breadcrumbs[1], { 3u, &locator, &state });
	}

	// endregion
}}
