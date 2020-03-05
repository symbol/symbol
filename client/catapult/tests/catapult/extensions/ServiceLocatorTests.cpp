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

#include "catapult/extensions/ServiceLocator.h"
#include "catapult/config/CatapultKeys.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS ServiceLocatorTests

	namespace {
		template<typename TAction>
		void RunLocatorTest(TAction action) {
			// Arrange:
			config::CatapultKeys keys;
			ServiceLocator locator(keys);

			// Act + Assert:
			action(locator);
		}
	}

	// region basic

	TEST(TEST_CLASS, CanCreateLocator) {
		// Act:
		config::CatapultKeys keys;
		ServiceLocator locator(keys);

		// Assert:
		EXPECT_EQ(&keys, &locator.keys());
		EXPECT_TRUE(locator.counters().empty());
		EXPECT_EQ(0u, locator.numServices());
	}

	// endregion

	// region services

	TEST(TEST_CLASS, ServiceThrowsWhenServiceIsNotRegistered) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService);

			// Act + Assert:
			EXPECT_THROW(locator.service<uint64_t>("bar"), catapult_invalid_argument);

			// Sanity:
			EXPECT_EQ(1u, locator.numServices());
		});
	}

	TEST(TEST_CLASS, ServiceReturnsNonNullWhenServiceIsRegisteredAndNotDestroyed) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService);

			// Act:
			auto pLocatedService = locator.service<uint64_t>("foo");

			// Assert:
			EXPECT_TRUE(!!pLocatedService);
			EXPECT_EQ(pService.get(), pLocatedService.get());
			EXPECT_EQ(1u, locator.numServices());
		});
	}

	TEST(TEST_CLASS, ServiceReturnsNullWhenServiceIsRegisteredAndDestroyed) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService);
			pService.reset();

			// Act:
			auto pLocatedService = locator.service<uint64_t>("foo");

			// Assert:
			EXPECT_FALSE(!!pLocatedService);
			EXPECT_EQ(1u, locator.numServices());
		});
	}

	TEST(TEST_CLASS, CannotRegisterSameServiceMultipleTimes) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService1 = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService1);

			// Act + Assert:
			auto pService2 = std::make_shared<uint64_t>(11);
			EXPECT_THROW(locator.registerService("foo", pService2), catapult_invalid_argument);
			EXPECT_EQ(1u, locator.numServices());
		});
	}

	TEST(TEST_CLASS, CanRegisterMultipleServices) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService1 = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService1);

			auto pService2 = std::make_shared<uint64_t>(11);
			locator.registerService("bar", pService2);

			// Act:
			auto pLocatedService1 = locator.service<uint64_t>("foo");
			auto pLocatedService2 = locator.service<uint64_t>("bar");

			// Assert:
			EXPECT_EQ(2u, locator.numServices());
			EXPECT_TRUE(!!pLocatedService1);
			EXPECT_EQ(pService1.get(), pLocatedService1.get());

			EXPECT_TRUE(!!pLocatedService2);
			EXPECT_EQ(pService2.get(), pLocatedService2.get());
		});
	}

	// endregion

	// region rooted services

	TEST(TEST_CLASS, ServiceReturnsNonNullWhenRootedServiceIsRegistered) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService = std::make_shared<uint64_t>(12);
			const auto& service = *pService;

			// - unlike for a regular service, the locator should extend the lifetime for a rooted service
			locator.registerRootedService("foo", pService);
			pService.reset();

			// Act:
			auto pLocatedService = locator.service<uint64_t>("foo");

			// Assert:
			EXPECT_TRUE(!!pLocatedService);
			EXPECT_EQ(&service, pLocatedService.get());
			EXPECT_EQ(1u, locator.numServices());

			// Sanity: locator owned and temporary
			EXPECT_EQ(2, pLocatedService.use_count());
		});
	}

	TEST(TEST_CLASS, CannotReregisterRegularServiceAsRootedService) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService1 = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService1);

			// Act + Assert:
			auto pService2 = std::make_shared<uint64_t>(11);
			EXPECT_THROW(locator.registerRootedService("foo", pService2), catapult_invalid_argument);
			EXPECT_EQ(1u, locator.numServices());

			// Sanity: locator does not extend lifetime of service that failed registration
			EXPECT_EQ(1, pService2.use_count());
		});
	}

	TEST(TEST_CLASS, CannotReregisterRootedServiceAsRegularService) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService1 = std::make_shared<uint64_t>(12);
			locator.registerRootedService("foo", pService1);

			// Act + Assert:
			auto pService2 = std::make_shared<uint64_t>(11);
			EXPECT_THROW(locator.registerService("foo", pService2), catapult_invalid_argument);
			EXPECT_EQ(1u, locator.numServices());

			// Sanity: locator continues to extend the lifetime of the original rooted service
			EXPECT_EQ(2, pService1.use_count());
		});
	}

	namespace {
		class BreadcrumbService {
		public:
			BreadcrumbService(const std::string& name, std::vector<std::string>& breadcrumbs)
					: m_name(name)
					, m_breadcrumbs(breadcrumbs)
			{}

			~BreadcrumbService() {
				m_breadcrumbs.push_back(m_name);
			}

		private:
			std::string m_name;
			std::vector<std::string>& m_breadcrumbs;
		};
	}

	TEST(TEST_CLASS, RootedServicesAreDestroyedInReverseRegistrationOrder) {
		// Arrange:
		std::vector<std::string> breadcrumbs;
		{
			config::CatapultKeys keys;
			ServiceLocator locator(keys);

			locator.registerRootedService("foo", std::make_shared<BreadcrumbService>("foo", breadcrumbs));
			locator.registerRootedService("bar", std::make_shared<BreadcrumbService>("bar", breadcrumbs));
			locator.registerRootedService("baz", std::make_shared<BreadcrumbService>("baz", breadcrumbs));

			// Act: destroy the locator
		}

		// Assert:
		EXPECT_EQ((std::vector<std::string>{ "baz", "bar", "foo" }), breadcrumbs);
	}

	// endregion

	// region counters

	TEST(TEST_CLASS, ServiceCounterReturnsSentinelValueWhenServiceIsNotRegistered) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			// - notice that registerService is not called
			locator.registerServiceCounter<uint64_t>("foo", "ALPHA", [](auto value) { return value; });

			// Act:
			const auto& counters = locator.counters();

			// Assert:
			ASSERT_EQ(1u, counters.size());
			EXPECT_EQ(utils::DiagnosticCounterId("ALPHA").value(), counters[0].id().value());
			EXPECT_EQ(ServiceLocator::Sentinel_Counter_Value, counters[0].value());
		});
	}

	TEST(TEST_CLASS, ServiceCounterReturnsServiceValueWhenServiceIsRegisteredAndNotDestroyed) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService);
			locator.registerServiceCounter<uint64_t>("foo", "ALPHA", [](auto value) { return value; });

			// Act:
			const auto& counters = locator.counters();

			// Assert:
			ASSERT_EQ(1u, counters.size());
			EXPECT_EQ(utils::DiagnosticCounterId("ALPHA").value(), counters[0].id().value());
			EXPECT_EQ(12u, counters[0].value());
		});
	}

	TEST(TEST_CLASS, ServiceCounterReturnsSentinelValueWhenServiceIsRegisteredAndDestroyed) {
		// Arrange:
		RunLocatorTest([](ServiceLocator& locator) {
			auto pService = std::make_shared<uint64_t>(12);
			locator.registerService("foo", pService);
			locator.registerServiceCounter<uint64_t>("foo", "ALPHA", [](auto value) { return value; });
			pService.reset();

			// Act:
			const auto& counters = locator.counters();

			// Assert:
			ASSERT_EQ(1u, counters.size());
			EXPECT_EQ(utils::DiagnosticCounterId("ALPHA").value(), counters[0].id().value());
			EXPECT_EQ(ServiceLocator::Sentinel_Counter_Value, counters[0].value());
		});
	}

	// endregion
}}
