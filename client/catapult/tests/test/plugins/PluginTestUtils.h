/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/plugins/PluginManager.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region EmptyPluginTraits

	/// Empty plugin traits.
	struct EmptyPluginTraits {
		static std::vector<model::EntityType> GetTransactionTypes() {
			return {};
		}

		static std::vector<std::string> GetCacheNames() {
			return {};
		}

		static std::vector<ionet::PacketType> GetNonDiagnosticPacketTypes() {
			return {};
		}

		static std::vector<ionet::PacketType> GetDiagnosticPacketTypes() {
			return {};
		}

		static std::vector<std::string> GetDiagnosticCounterNames() {
			return {};
		}

		static std::vector<std::string> GetStatelessValidatorNames() {
			return {};
		}

		static std::vector<std::string> GetStatefulValidatorNames() {
			return {};
		}

		static std::vector<std::string> GetObserverNames() {
			return {};
		}

		static std::vector<std::string> GetPermanentObserverNames() {
			return {};
		}
	};

	// endregion

	// region tests

	/// Asserts that transactions have been registered.
	template<typename TTraits>
	void AssertAppropriateTransactionsAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](const auto& manager) {
			// Act:
			const auto& transactionRegistry = manager.transactionRegistry();

			// Assert:
			auto expectedTypes = TTraits::GetTransactionTypes();
			EXPECT_EQ(expectedTypes.size(), transactionRegistry.size());

			for (const auto type : expectedTypes) {
				CATAPULT_LOG(debug) << "checking type " << type;
				EXPECT_TRUE(!!transactionRegistry.findPlugin(type));
			}
		});
	}

	/// Asserts that caches have been registered.
	template<typename TTraits>
	void AssertAppropriateCachesAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](auto& manager) {
			// Act:
			auto cache = manager.createCache();

			// - extract the storage names
			std::vector<std::string> storageNames;
			for (const auto& pStorage : cache.storages())
				storageNames.push_back(pStorage->name());

			// Assert:
			EXPECT_EQ(TTraits::GetCacheNames(), storageNames);
		});
	}

	/// Asserts that (non-diagnostic) handlers have been registered.
	template<typename TTraits>
	void AssertAppropriateNonDiagnosticHandlersAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](auto& manager) {
			// Act:
			ionet::ServerPacketHandlers handlers;
			manager.addHandlers(handlers, manager.createCache());

			// Assert:
			auto expectedTypes = TTraits::GetNonDiagnosticPacketTypes();
			EXPECT_EQ(expectedTypes.size(), handlers.size());

			for (const auto type : expectedTypes) {
				CATAPULT_LOG(debug) << "checking type " << type;
				ionet::Packet packet;
				packet.Type = type;
				EXPECT_TRUE(handlers.canProcess(packet));
			}
		});
	}

	/// Asserts that diagnostic handlers have been registered.
	template<typename TTraits>
	void AssertAppropriateDiagnosticHandlersAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](auto& manager) {
			// Act:
			ionet::ServerPacketHandlers handlers;
			manager.addDiagnosticHandlers(handlers, manager.createCache());

			// Assert:
			auto expectedTypes = TTraits::GetDiagnosticPacketTypes();
			EXPECT_EQ(expectedTypes.size(), handlers.size());

			for (const auto type : expectedTypes) {
				CATAPULT_LOG(debug) << "checking type " << type;
				ionet::Packet packet;
				packet.Type = type;
				EXPECT_TRUE(handlers.canProcess(packet));
			}
		});
	}

	/// Asserts that diagnostic counters have been registered.
	template<typename TTraits>
	void AssertAppropriateDiagnosticCountersAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](auto& manager) {
			// Act:
			std::vector<utils::DiagnosticCounter> counters;
			manager.addDiagnosticCounters(counters, manager.createCache());

			// - extract the counter names
			std::vector<std::string> counterNames;
			for (const auto& counter : counters)
				counterNames.push_back(counter.id().name());

			// Assert:
			EXPECT_EQ(TTraits::GetDiagnosticCounterNames(), counterNames);
		});
	}

	/// Asserts that stateless validators have been registered.
	template<typename TTraits>
	void AssertAppropriateStatelessValidatorsAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](const auto& manager) {
			// Act:
			auto pValidator = manager.createStatelessValidator();

			// Assert:
			EXPECT_EQ(TTraits::GetStatelessValidatorNames(), pValidator->names());
		});
	}

	/// Asserts that stateful validators have been registered.
	template<typename TTraits>
	void AssertAppropriateStatefulValidatorsAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](const auto& manager) {
			// Act:
			auto pValidator = manager.createStatefulValidator();

			// Assert:
			EXPECT_EQ(TTraits::GetStatefulValidatorNames(), pValidator->names());
		});
	}

	/// Asserts that permanent and transient observers have been registered.
	template<typename TTraits>
	void AssertAppropriateObserversAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](const auto& manager) {
			// Act:
			auto pObserver = manager.createObserver();

			// Assert:
			EXPECT_EQ(TTraits::GetObserverNames(), pObserver->names());
		});
	}

	/// Asserts that permanent observers have been registered.
	template<typename TTraits>
	void AssertAppropriatePermanentObserversAreRegistered() {
		// Arrange:
		TTraits::RunTestAfterRegistration([](const auto& manager) {
			// Act:
			auto pObserver = manager.createPermanentObserver();

			// Assert:
			EXPECT_EQ(TTraits::GetPermanentObserverNames(), pObserver->names());
		});
	}

	// endregion

#define MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::Assert##TEST_NAME<TEST_TRAITS>(); }

#define DEFINE_PLUGIN_TESTS(TEST_CLASS, TEST_TRAITS) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateTransactionsAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateCachesAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateNonDiagnosticHandlersAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateDiagnosticHandlersAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateDiagnosticCountersAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateStatelessValidatorsAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateStatefulValidatorsAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriateObserversAreRegistered) \
	MAKE_PLUGIN_TEST(TEST_CLASS, TEST_TRAITS, AppropriatePermanentObserversAreRegistered)
}}
