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

#include "partialtransaction/src/chain/PtValidator.h"
#include "plugins/txes/aggregate/src/model/AggregateNotifications.h"
#include "plugins/txes/aggregate/src/validators/Results.h"
#include "catapult/model/WeakCosignedTransactionInfo.h"
#include "catapult/plugins/PluginManager.h"
#include "partialtransaction/tests/test/AggregateTransactionTestUtils.h"
#include "tests/test/other/mocks/MockCapturingNotificationValidator.h"
#include "tests/test/plugins/PluginManagerFactory.h"
#include "tests/TestHarness.h"

using namespace catapult::validators;

namespace catapult { namespace chain {

#define TEST_CLASS PtValidatorTests

	namespace {
		using StatefulValidatorPointer = std::unique_ptr<mocks::MockCapturingStatefulNotificationValidator>;
		using StatefulValidatorRawPointers = std::vector<const StatefulValidatorPointer::element_type*>;

		using StatelessValidatorPointer = std::unique_ptr<mocks::MockCapturingStatelessNotificationValidator>;
		using StatelessValidatorRawPointers = std::vector<const StatelessValidatorPointer::element_type*>;

		constexpr auto Default_Block_Time = Timestamp(987);

		enum class ValidatorType { Stateful, Stateless };

		class ValidationResultOptions {
		public:
			ValidationResultOptions(ValidationResult result)
					: m_result(result)
					, m_errorType(ValidatorType::Stateful)
					, m_triggerOnSpecificNotificationType(false)
			{}

			ValidationResultOptions(ValidatorType errorType, model::NotificationType triggerType)
					: m_result(ValidationResult::Failure)
					, m_errorType(errorType)
					, m_triggerOnSpecificNotificationType(true)
					, m_triggerType(triggerType)
			{}

		public:
			ValidationResult result() const {
				return m_result;
			}

			bool isStatefulResult() const {
				return ValidatorType::Stateful == m_errorType;
			}

			template<typename TValidator>
			void setResult(TValidator& validator) const {
				if (m_triggerOnSpecificNotificationType)
					validator.setResult(m_result, m_triggerType);
				else
					validator.setResult(m_result);
			}

		private:
			ValidationResult m_result;
			ValidatorType m_errorType;
			bool m_triggerOnSpecificNotificationType;
			model::NotificationType m_triggerType;
		};

		class TestContext {
		public:
			explicit TestContext(const ValidationResultOptions& options)
					: m_cache(test::CreateEmptyCatapultCache())
					, m_pluginManager(test::CreatePluginManager()) {
				// Arrange: register mock support (for validatePartial)
				auto pluginOptionFlags = mocks::PluginOptionFlags::Publish_Custom_Notifications;
				m_pluginManager.addTransactionSupport(mocks::CreateMockTransactionPlugin(pluginOptionFlags));

				// - register a validator that will return the desired result
				if (options.isStatefulResult()) {
					m_pluginManager.addStatefulValidatorHook([&options, &validators = m_statefulValidators](auto& builder) {
						auto pValidator = std::make_unique<StatefulValidatorPointer::element_type>();
						options.setResult(*pValidator);
						validators.push_back(pValidator.get());
						builder.add(std::move(pValidator));
					});
				} else {
					m_pluginManager.addStatelessValidatorHook([&options, &validators = m_statelessValidators](auto& builder) {
						auto pValidator = std::make_unique<StatelessValidatorPointer::element_type>();
						options.setResult(*pValidator);
						validators.push_back(pValidator.get());
						builder.add(std::move(pValidator));
					});
				}

				m_pValidator = CreatePtValidator(m_cache, []() { return Default_Block_Time; }, m_pluginManager);
			}

		public:
			const auto& validator() {
				return *m_pValidator;
			}

			const auto& subValidatorAt(size_t index) {
				return *m_statefulValidators[index];
			}

			const auto& subStatelessValidatorAt(size_t index) {
				return *m_statelessValidators[index];
			}

		private:
			cache::CatapultCache m_cache;
			plugins::PluginManager m_pluginManager;
			std::unique_ptr<PtValidator> m_pValidator;
			StatefulValidatorRawPointers m_statefulValidators;
			StatelessValidatorRawPointers m_statelessValidators;
		};
	}

	// region validatePartial

	namespace {
		struct ValidatePartialResult {
			bool IsValid;
			bool IsShortCircuited;
		};

		template<typename TCapturedParams>
		void ValidateTransactionNotifications(
				const TCapturedParams& capturedParams,
				const Hash256& expectedHash,
				Timestamp expectedDeadline,
				const std::string& description) {
			// Assert:
			auto i = 0u;
			auto numMatches = 0u;
			for (const auto& params : capturedParams) {
				++i;
				if (!params.TransactionNotificationInfo.IsSet)
					continue;

				auto message = description + " - notification at " + std::to_string(i);
				EXPECT_EQ(expectedHash, params.TransactionNotificationInfo.TransactionHash) << message;
				EXPECT_EQ(expectedDeadline, params.TransactionNotificationInfo.Deadline) << message;
				++numMatches;
			}

			EXPECT_LE(0u, numMatches) << description;
		}

		template<typename TNotificationTypesConsumer>
		void RunValidatePartialTest(
				const ValidationResultOptions& validationResultOptions,
				bool isValid,
				TNotificationTypesConsumer notificationTypesConsumer) {
			// Arrange:
			TestContext context(validationResultOptions);
			const auto& validator = context.validator();
			const auto& notificationValidator = context.subValidatorAt(0); // partial

			// Act: validatePartial does not filter transactions even though, in practice, it will only be called with aggregates
			auto pTransaction = mocks::CreateMockTransaction(0);
			auto transactionHash = test::GenerateRandomByteArray<Hash256>();
			auto result = validator.validatePartial(model::WeakEntityInfoT<model::Transaction>(*pTransaction, transactionHash));

			// Assert: when valid, raw result should always be success (even when validationResult is suppressed failure)
			EXPECT_EQ(isValid, result.Normalized);
			EXPECT_EQ(isValid ? ValidationResult::Success : validationResultOptions.result(), result.Raw);

			// - short circuiting on failure
			notificationTypesConsumer(notificationValidator.notificationTypes());

			// - correct timestamp was passed to validator
			ASSERT_LE(1u, notificationValidator.params().size());
			EXPECT_EQ(Default_Block_Time, notificationValidator.params()[0].BlockTime);

			// - correct transaction information is passed down (if validation wasn't short circuited)
			if (notificationValidator.notificationTypes().size() > 1)
				ValidateTransactionNotifications(notificationValidator.params(), transactionHash, pTransaction->Deadline, "basic");
		}

		void RunValidatePartialTest(const ValidationResultOptions& validationResultOptions, const ValidatePartialResult& expectedResult) {
			// Arrange:
			RunValidatePartialTest(validationResultOptions, expectedResult.IsValid, [&expectedResult](const auto& notificationTypes) {
				// Assert:
				if (!expectedResult.IsShortCircuited) {
					ASSERT_EQ(6u, notificationTypes.size());
					EXPECT_EQ(model::Core_Entity_Notification, notificationTypes[0]);
					EXPECT_EQ(model::Core_Transaction_Notification, notificationTypes[1]);
					EXPECT_EQ(model::Core_Transaction_Deadline_Notification, notificationTypes[2]);
					EXPECT_EQ(model::Core_Transaction_Fee_Notification, notificationTypes[3]);
					EXPECT_EQ(model::Core_Balance_Debit_Notification, notificationTypes[4]);
					EXPECT_EQ(model::Core_Signature_Notification, notificationTypes[5]);
				} else {
					ASSERT_EQ(1u, notificationTypes.size());
					EXPECT_EQ(model::Core_Entity_Notification, notificationTypes[0]);
				}
			});
		}
	}

	TEST(TEST_CLASS, ValidatePartialMapsSuccessToTrue) {
		// Assert:
		RunValidatePartialTest(ValidationResult::Success, { true, false });
	}

	TEST(TEST_CLASS, ValidatePartialMapsNeutralToFalse) {
		// Assert:
		RunValidatePartialTest(ValidationResult::Neutral, { false, false });
	}

	TEST(TEST_CLASS, ValidatePartialMapsGenericFailureToFalse) {
		// Assert:
		RunValidatePartialTest(ValidationResult::Failure, { false, true });
	}

	TEST(TEST_CLASS, ValidatePartialMapsFailureAggregateIneligibleCosignersToFalse) {
		// Assert:
		RunValidatePartialTest(Failure_Aggregate_Ineligible_Cosigners, { false, true });
	}

	TEST(TEST_CLASS, ValidatePartialMapsFailureAggregateMissingCosignersToTrue) {
		// Assert:
		RunValidatePartialTest(Failure_Aggregate_Missing_Cosigners, { true, false });
	}

	TEST(TEST_CLASS, ValidatePartialMapsBasicStatefulFailureToFalse) {
		// Arrange:
		auto options = ValidationResultOptions{ ValidatorType::Stateful, model::Core_Transaction_Notification };
		RunValidatePartialTest(options, false, [](const auto& notificationTypes) {
			// Assert:
			ASSERT_EQ(2u, notificationTypes.size());
			EXPECT_EQ(model::Core_Entity_Notification, notificationTypes[0]);
			EXPECT_EQ(model::Core_Transaction_Notification, notificationTypes[1]);
		});
	}

	TEST(TEST_CLASS, ValidatePartialMapsCustomStatefulFailureToTrue) {
		// Assert: custom stateful validators are bypassed
		RunValidatePartialTest({ ValidatorType::Stateful, mocks::Mock_Validator_1_Notification }, { true, false });
	}

	namespace {
		template<typename TNotificationTypesConsumer>
		void RunInvalidStatelessValidatePartialTest(
				model::NotificationType notificationType,
				TNotificationTypesConsumer notificationTypesConsumer) {
			// Arrange:
			ValidationResultOptions validationResultOptions{ ValidatorType::Stateless, notificationType };
			TestContext context(validationResultOptions);
			const auto& validator = context.validator();
			const auto& basicNotificationValidator = context.subStatelessValidatorAt(0); // partial (basic)
			const auto& customNotificationValidator = context.subStatelessValidatorAt(1); // partial (custom)

			// Act: validatePartial does not filter transactions even though, in practice, it will only be called with aggregates
			auto pTransaction = mocks::CreateMockTransaction(0);
			auto transactionHash = test::GenerateRandomByteArray<Hash256>();
			auto result = validator.validatePartial(model::WeakEntityInfoT<model::Transaction>(*pTransaction, transactionHash));

			// Assert:
			EXPECT_FALSE(result.Normalized);
			EXPECT_EQ(validationResultOptions.result(), result.Raw);

			// - merge notification types from both validators
			auto allNotificationTypes = basicNotificationValidator.notificationTypes();
			auto customNotificationTypes = customNotificationValidator.notificationTypes();
			allNotificationTypes.insert(allNotificationTypes.end(), customNotificationTypes.cbegin(), customNotificationTypes.cend());
			notificationTypesConsumer(allNotificationTypes);

			// - correct transaction information is passed down
			ValidateTransactionNotifications(basicNotificationValidator.params(), transactionHash, pTransaction->Deadline, "basic");
		}
	}

	TEST(TEST_CLASS, ValidatePartialMapsBasicStatelessFailureToFalse) {
		// Arrange:
		RunInvalidStatelessValidatePartialTest(model::Core_Transaction_Notification, [](const auto& notificationTypes) {
			// Assert:
			ASSERT_EQ(2u, notificationTypes.size());
			EXPECT_EQ(model::Core_Entity_Notification, notificationTypes[0]);
			EXPECT_EQ(model::Core_Transaction_Notification, notificationTypes[1]);
		});
	}

	TEST(TEST_CLASS, ValidatePartialMapsCustomStatelessFailureToFalse) {
		// Arrange:
		RunInvalidStatelessValidatePartialTest(mocks::Mock_Validator_1_Notification, [](const auto& notificationTypes) {
			// Assert:
			ASSERT_EQ(7u, notificationTypes.size());
			EXPECT_EQ(model::Core_Entity_Notification, notificationTypes[0]);
			EXPECT_EQ(model::Core_Transaction_Notification, notificationTypes[1]);
			EXPECT_EQ(model::Core_Transaction_Deadline_Notification, notificationTypes[2]);
			EXPECT_EQ(model::Core_Transaction_Fee_Notification, notificationTypes[3]);
			EXPECT_EQ(model::Core_Balance_Debit_Notification, notificationTypes[4]);
			EXPECT_EQ(model::Core_Signature_Notification, notificationTypes[5]);
			EXPECT_EQ(mocks::Mock_Validator_1_Notification, notificationTypes[6]);
		});
	}

	// endregion

	// region validateCosigners

	namespace {
		struct ValidateCosignersResult {
			CosignersValidationResult Result;
			bool IsShortCircuited;
		};

		auto CreateAggregateTransaction(uint8_t numTransactions) {
			return test::CreateAggregateTransaction(numTransactions).pTransaction;
		}

		void RunValidateCosignersTest(ValidationResult validationResult, const ValidateCosignersResult& expectedResult) {
			// Arrange:
			TestContext context(validationResult);
			const auto& validator = context.validator();
			const auto& notificationValidator = context.subValidatorAt(1); // cosigners

			// Act:
			auto pTransaction = CreateAggregateTransaction(2);
			auto cosignatures = test::GenerateRandomDataVector<model::Cosignature>(3);
			auto result = validator.validateCosigners({ pTransaction.get(), &cosignatures });

			// Assert:
			EXPECT_EQ(expectedResult.Result, result.Normalized);
			EXPECT_EQ(validationResult, result.Raw);

			// - short circuiting on failure
			const auto& notificationTypes = notificationValidator.notificationTypes();
			if (!expectedResult.IsShortCircuited) {
				ASSERT_EQ(3u, notificationTypes.size());
				EXPECT_EQ(model::Aggregate_Cosignatures_Notification, notificationTypes[0]);
				EXPECT_EQ(model::Aggregate_EmbeddedTransaction_Notification, notificationTypes[1]);
				EXPECT_EQ(model::Aggregate_EmbeddedTransaction_Notification, notificationTypes[2]);
			} else {
				ASSERT_EQ(1u, notificationTypes.size());
				EXPECT_EQ(model::Aggregate_Cosignatures_Notification, notificationTypes[0]);
			}

			// - correct timestamp was passed to validator
			ASSERT_LE(1u, notificationValidator.params().size());
			EXPECT_EQ(Default_Block_Time, notificationValidator.params()[0].BlockTime);
		}
	}

	TEST(TEST_CLASS, ValidateCosignersMapsSuccessToSuccess) {
		// Assert:
		RunValidateCosignersTest(ValidationResult::Success, { CosignersValidationResult::Success, false });
	}

	TEST(TEST_CLASS, ValidateCosignersMapsNeutralToFailure) {
		// Assert:
		RunValidateCosignersTest(ValidationResult::Neutral, { CosignersValidationResult::Failure, false });
	}

	TEST(TEST_CLASS, ValidateCosignersMapsGenericFailureToFailure) {
		// Assert:
		RunValidateCosignersTest(ValidationResult::Failure, { CosignersValidationResult::Failure, true });
	}

	TEST(TEST_CLASS, ValidateCosignersMapsFailureAggregateIneligibleCosignersToIneligible) {
		// Assert:
		RunValidateCosignersTest(Failure_Aggregate_Ineligible_Cosigners, { CosignersValidationResult::Ineligible, true });
	}

	TEST(TEST_CLASS, ValidateCosignersMapsFailureAggregateMissingCosignersToMissing) {
		// Assert:
		RunValidateCosignersTest(Failure_Aggregate_Missing_Cosigners, { CosignersValidationResult::Missing, true });
	}

	// endregion
}}
