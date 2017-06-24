#include "catapult/local/NotificationValidatorAdapter.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::validators;

namespace catapult { namespace local {

#define TEST_CLASS NotificationValidatorAdapterTests

	namespace {
		ValidationResult ValidateEntity(
				const stateless::EntityValidator& validator,
				const model::VerifiableEntity& entity) {
			Hash256 hash;
			return validator.validate(model::WeakEntityInfo(entity, hash));
		}

		class MockNotificationValidator : public stateless::NotificationValidator {
		public:
			explicit MockNotificationValidator(const std::string& name, ValidationResult result)
					: m_name(name)
					, m_result(result)
					, m_numValidateCalls(0)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			ValidationResult validate(const model::Notification& notification) const override {
				++m_numValidateCalls;
				m_notificationTypes.push_back(notification.Type);

				if (model::Core_Signature_Notification == notification.Type)
					m_signerKeys.push_back(static_cast<const model::SignatureNotification&>(notification).Signer);

				return m_result;
			}

		public:
			size_t numValidateCalls() const {
				return m_numValidateCalls;
			}

			const auto& notificationTypes() const {
				return m_notificationTypes;
			}

			const auto& signerKeys() const {
				return m_signerKeys;
			}

		private:
			std::string m_name;
			ValidationResult m_result;
			mutable size_t m_numValidateCalls;
			mutable std::vector<model::NotificationType> m_notificationTypes;
			mutable std::vector<Key> m_signerKeys;
		};

		template<typename TRunTestFunc>
		void RunTest(ValidationResult result, TRunTestFunc runTest) {
			// Arrange:
			auto pRegistry = mocks::CreateDefaultTransactionRegistry(mocks::PluginOptionFlags::Publish_Custom_Notifications);
			auto pValidator = std::make_unique<MockNotificationValidator>("alpha", result);
			const auto& validator = *pValidator;
			auto pAdapter = std::make_unique<NotificationValidatorAdapter>(*pRegistry, std::move(pValidator));

			// Act + Assert:
			runTest(*pAdapter, validator);
		}
	}

	TEST(TEST_CLASS, CanCreateAdapter) {
		// Arrange:
		RunTest(ValidationResult::Success, [](const auto& adapter, const auto&) {
			// Assert:
			EXPECT_EQ("alpha", adapter.name());
		});
	}

	TEST(TEST_CLASS, ExtractsAndForwardsNotificationsFromEntity) {
		// Arrange:
		RunTest(ValidationResult::Success, [](const auto& adapter, const auto& validator) {
			// Act:
			auto pTransaction = mocks::CreateMockTransaction(0);
			ValidateEntity(adapter, *pTransaction);

			// Assert: the mock transaction plugin sends additional public key notification and 6 custom notifications
			//         (notice that only 4/6 are raised on validator channel)
			ASSERT_EQ(3u + 4u, validator.notificationTypes().size());
			EXPECT_EQ(model::Core_Entity_Notification, validator.notificationTypes()[0]);
			EXPECT_EQ(model::Core_Transaction_Notification, validator.notificationTypes()[1]);
			EXPECT_EQ(model::Core_Signature_Notification, validator.notificationTypes()[2]);

			// - mock transaction notifications
			EXPECT_EQ(mocks::Mock_Validator_1_Notification, validator.notificationTypes()[3]);
			EXPECT_EQ(mocks::Mock_All_1_Notification, validator.notificationTypes()[4]);
			EXPECT_EQ(mocks::Mock_Validator_2_Notification, validator.notificationTypes()[5]);
			EXPECT_EQ(mocks::Mock_All_2_Notification, validator.notificationTypes()[6]);

			// - spot check the signer keys as a proxy for verifying data integrity
			ASSERT_EQ(1u, validator.signerKeys().size());
			EXPECT_EQ(pTransaction->Signer, validator.signerKeys()[0]);
		});
	}

	namespace {
		void AssertMockTransactionValidation(ValidationResult expectedResult, size_t expectedNumValidateCalls) {
			// Arrange:
			RunTest(expectedResult, [&](const auto& adapter, const auto& validator) {
				// Act:
				auto pTransaction = mocks::CreateMockTransaction(0);
				auto result = ValidateEntity(adapter, *pTransaction);

				// Assert:
				EXPECT_EQ(expectedResult, result);
				EXPECT_EQ(expectedNumValidateCalls, validator.numValidateCalls());
			});
		}
	}

	TEST(TEST_CLASS, DelegatesWhenTypeMatches_Success) {
		// Assert: all five notifications should be processed
		AssertMockTransactionValidation(ValidationResult::Success, 7);
	}

	TEST(TEST_CLASS, DelegatesWhenTypeMatches_Neutral) {
		// Assert: all five notifications should be processed
		AssertMockTransactionValidation(ValidationResult::Neutral, 7);
	}

	TEST(TEST_CLASS, DelegatesWhenTypeMatches_Failure) {
		// Assert: first failure short-circuits
		AssertMockTransactionValidation(ValidationResult::Failure, 1);
	}
}}
