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

#include "partialtransaction/src/chain/JointValidator.h"
#include "catapult/plugins/PluginManager.h"
#include "tests/test/other/mocks/MockCapturingNotificationValidator.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/TestHarness.h"

using namespace catapult::validators;

namespace catapult { namespace chain {

#define TEST_CLASS JointValidatorTests

	namespace {
		// region TestContext

		constexpr auto Cache_Height = Height(25);
		constexpr auto Default_Block_Time = Timestamp(987);
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(17);

		enum class FailureMode { Default, Suppress };

		model::BlockChainConfiguration CreateConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			return config;
		}

		class TestContext {
		private:
			using StatelessValidatorPointer = std::unique_ptr<mocks::MockCapturingStatelessNotificationValidator>;
			using StatefulValidatorPointer = std::unique_ptr<mocks::MockCapturingStatefulNotificationValidator>;

		public:
			TestContext() : TestContext("stateless", "stateful")
			{}

			TestContext(const std::string& statelessName, const std::string& statefulName)
					: m_statelessName(statelessName)
					, m_statefulName(statefulName)
					, m_statelessResult(ValidationResult::Success)
					, m_statefulResult(ValidationResult::Success)
					, m_cache(test::CreateCatapultCacheWithMarkerAccount())
					, m_pluginManager(test::CreatePluginManager(CreateConfiguration())) {
				// set custom cache height
				auto cacheDelta = m_cache.createDelta();
				m_cache.commit(Cache_Height);
			}

		public:
			void setStatelessResult(ValidationResult result) {
				m_statelessResult = result;
			}

			void setStatefulResult(ValidationResult result) {
				m_statefulResult = result;
			}

		public:
			std::unique_ptr<const stateless::NotificationValidator> create(FailureMode failureMode = FailureMode::Default) {
				m_pluginManager.addMosaicResolver([](const auto& cache, const auto& unresolved, auto& resolved) {
					resolved = MosaicId(unresolved.unwrap() * (test::IsMarkedCache(cache) ? 2 : 0));
					return true;
				});

				m_pluginManager.addStatelessValidatorHook([this](auto& builder) {
					builder.add(this->createStatelessValidator());
				});

				m_pluginManager.addStatefulValidatorHook([this](auto& builder) {
					builder.add(this->createStatefulValidator());
				});

				return CreateJointValidator(m_cache, []() { return Default_Block_Time; }, m_pluginManager, [failureMode](auto) {
					return FailureMode::Suppress == failureMode;
				});
			}

		private:
			StatelessValidatorPointer createStatelessValidator() {
				auto pValidator = std::make_unique<StatelessValidatorPointer::element_type>(m_statelessName);
				pValidator->setResult(m_statelessResult);
				m_pStatelessValidator = pValidator.get();
				return pValidator;
			}

			StatefulValidatorPointer createStatefulValidator() {
				auto pValidator = std::make_unique<StatefulValidatorPointer::element_type>(m_statefulName);
				pValidator->setResult(m_statefulResult);
				m_pStatefulValidator = pValidator.get();
				return pValidator;
			}

		public:
			void assertStatelessShortCircuit(const model::Notification& notification) {
				// Assert: stateless should be called
				assertStatelessSingle(notification);

				// - stateful should be bypassed
				EXPECT_EQ(0u, m_pStatefulValidator->params().size());
			}

			void assertStatelessAndStateful(const model::Notification& notification) {
				// Assert: stateless should be called
				assertStatelessSingle(notification);

				// - stateful should be called
				assertStatefulSingle(notification);
			}

		private:
			void assertStatelessSingle(const model::Notification& notification) {
				ASSERT_EQ(1u, m_pStatelessValidator->params().size());

				const auto& params = m_pStatelessValidator->params()[0];
				EXPECT_EQ(&notification, &params.Notification);
			}

			void assertStatefulSingle(const model::Notification& notification) {
				ASSERT_EQ(1u, m_pStatefulValidator->params().size());

				const auto& params = m_pStatefulValidator->params()[0];
				EXPECT_EQ(&notification, &params.Notification);

				EXPECT_EQ(Cache_Height, params.Height);
				EXPECT_EQ(Default_Block_Time, params.BlockTime);
				EXPECT_EQ(Network_Identifier, params.NetworkIdentifier);
				EXPECT_TRUE(params.IsMarkedCache);

				// - check that context includes wired up resolvers
				EXPECT_EQ(MosaicId(2), params.ResolvedMosaicIdOne);
			}

		private:
			std::string m_statelessName;
			std::string m_statefulName;
			ValidationResult m_statelessResult;
			ValidationResult m_statefulResult;

			cache::CatapultCache m_cache;
			plugins::PluginManager m_pluginManager;

			const StatelessValidatorPointer::element_type* m_pStatelessValidator; // only valid after create
			const StatefulValidatorPointer::element_type* m_pStatefulValidator; // only valid after create
		};

		// endregion
	}

	// region basic

	TEST(TEST_CLASS, CanCreateJointValidator) {
		// Arrange:
		TestContext context("alpha-less", "beta-ful");
		auto pValidator = context.create();

		// Act:
		auto name = pValidator->name();

		// Assert:
		EXPECT_EQ("JointValidator ({ alpha-less }, { beta-ful })", name);
	}

	// endregion

	// region validate

	namespace {
		void AssertJointValidationResult(
				ValidationResult expectedResult,
				ValidationResult statelessResult,
				ValidationResult statefulResult) {
			// Arrange:
			TestContext context;
			context.setStatelessResult(statelessResult);
			context.setStatefulResult(statefulResult);
			auto pValidator = context.create();

			// Act:
			model::AccountPublicKeyNotification notification(test::GenerateRandomByteArray<Key>());
			auto result = pValidator->validate(notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
			context.assertStatelessAndStateful(notification);
		}
	}

	TEST(TEST_CLASS, StatelessSuccessAndStatefulSuccessYieldSuccess) {
		AssertJointValidationResult(ValidationResult::Success, ValidationResult::Success, ValidationResult::Success);
	}

	TEST(TEST_CLASS, StatelessNeutralDominatesStatefulSuccess) {
		AssertJointValidationResult(ValidationResult::Neutral, ValidationResult::Success, ValidationResult::Neutral);
	}

	TEST(TEST_CLASS, StatefulNeutralDominatesStatelessSuccess) {
		AssertJointValidationResult(ValidationResult::Neutral, ValidationResult::Success, ValidationResult::Neutral);
	}

	TEST(TEST_CLASS, StatefulFailureDominatesStatelessNeutral) {
		AssertJointValidationResult(ValidationResult::Failure, ValidationResult::Neutral, ValidationResult::Failure);
	}

	TEST(TEST_CLASS, StatelessShortCircuitsOnFailure) {
		// Arrange:
		TestContext context;
		context.setStatelessResult(ValidationResult::Failure);
		auto pValidator = context.create();

		// Act:
		model::AccountPublicKeyNotification notification(test::GenerateRandomByteArray<Key>());
		auto result = pValidator->validate(notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Failure, result);
		context.assertStatelessShortCircuit(notification);
	}

	TEST(TEST_CLASS, FailuresCanBeSuppressedAcrossAllValidators) {
		// Arrange:
		TestContext context;
		context.setStatelessResult(ValidationResult::Failure);
		context.setStatefulResult(ValidationResult::Failure);
		auto pValidator = context.create(FailureMode::Suppress);

		// Act:
		model::AccountPublicKeyNotification notification(test::GenerateRandomByteArray<Key>());
		auto result = pValidator->validate(notification);

		// Assert: the failures were suppressed
		EXPECT_EQ(ValidationResult::Success, result);
		context.assertStatelessAndStateful(notification);
	}

	// endregion
}}
