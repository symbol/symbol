#include "partialtransaction/src/chain/JointValidator.h"
#include "catapult/plugins/PluginManager.h"
#include "tests/test/other/mocks/MockCapturingNotificationValidator.h"
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
					, m_cache(test::CreateCatapultCacheWithMarkerAccount()) {
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
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Network.Identifier = Network_Identifier;
				plugins::PluginManager pluginManager(config);

				pluginManager.addStatelessValidatorHook([this](auto& builder) {
					builder.add(this->createStatelessValidator());
				});

				pluginManager.addStatefulValidatorHook([this](auto& builder) {
					builder.add(this->createStatefulValidator());
				});

				return CreateJointValidator(m_cache, []() { return Default_Block_Time; }, pluginManager, [failureMode](auto) {
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
			}

		private:
			std::string m_statelessName;
			std::string m_statefulName;
			ValidationResult m_statelessResult;
			ValidationResult m_statefulResult;

			cache::CatapultCache m_cache;

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
			model::AccountPublicKeyNotification notification(test::GenerateRandomData<Key_Size>());
			auto result = pValidator->validate(notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
			context.assertStatelessAndStateful(notification);
		}
	}

	TEST(TEST_CLASS, StatelessSuccessAndStatefulSuccessYieldSuccess) {
		// Assert:
		AssertJointValidationResult(ValidationResult::Success, ValidationResult::Success, ValidationResult::Success);
	}

	TEST(TEST_CLASS, StatelessNeutralDominatesStatefulSuccess) {
		// Assert:
		AssertJointValidationResult(ValidationResult::Neutral, ValidationResult::Success, ValidationResult::Neutral);
	}

	TEST(TEST_CLASS, StatefulNeutralDominatesStatelessSuccess) {
		// Assert:
		AssertJointValidationResult(ValidationResult::Neutral, ValidationResult::Success, ValidationResult::Neutral);
	}

	TEST(TEST_CLASS, StatefulFailureDominatesStatelessNeutral) {
		// Assert:
		AssertJointValidationResult(ValidationResult::Failure, ValidationResult::Neutral, ValidationResult::Failure);
	}

	TEST(TEST_CLASS, StatelessShortCircuitsOnFailure) {
		// Arrange:
		TestContext context;
		context.setStatelessResult(ValidationResult::Failure);
		auto pValidator = context.create();

		// Act:
		model::AccountPublicKeyNotification notification(test::GenerateRandomData<Key_Size>());
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
		model::AccountPublicKeyNotification notification(test::GenerateRandomData<Key_Size>());
		auto result = pValidator->validate(notification);

		// Assert: the failures were suppressed
		EXPECT_EQ(ValidationResult::Success, result);
		context.assertStatelessAndStateful(notification);
	}

	// endregion
}}
