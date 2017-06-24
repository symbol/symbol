#pragma once
#include "ValidationPolicyTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"

namespace catapult { namespace validators {

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(ValidateInvokesValidateOnEachContainedValidator) {
		// Arrange:
		auto states = test::CreateCounterStates<typename TTraits::ValidatorStateVector>(2);
		typename TTraits::PairVector pairs{
			std::make_pair("Success", ValidationResult::Success),
			std::make_pair("Neutral", ValidationResult::Neutral),
		};

		auto pValidator = test::CreateValidator(pairs, states);
		auto dispatcher = TTraits::CreateDispatcher();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		TTraits::Dispatch(*pValidator, dispatcher, entityInfos.toVector());

		// Assert:
		EXPECT_EQ(2u, test::GetNumSubValidators(*pValidator));
		EXPECT_EQ(1u, test::CounterAt(states, 0));
		EXPECT_EQ(1u, test::CounterAt(states, 1));
	}

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(ValidateInvokesValidateOnEachEntity) {
		// Arrange:
		auto states = test::CreateCounterStates<typename TTraits::ValidatorStateVector>(1);
		typename TTraits::PairVector pairs{
			std::make_pair("Success", ValidationResult::Success)
		};

		auto pValidator = test::CreateValidator(pairs, states);
		auto dispatcher = TTraits::CreateDispatcher();

		// Act:
		auto entityInfos = test::CreateEntityInfos(3);
		TTraits::Dispatch(*pValidator, dispatcher, entityInfos.toVector());

		// Assert:
		EXPECT_EQ(1u, test::GetNumSubValidators(*pValidator));
		EXPECT_EQ(3u, test::CounterAt(states, 0));
	}

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(CanCallValidateMultipleTimesConsecutively) {
		// Arrange:
		auto states = test::CreateCounterStates<typename TTraits::ValidatorStateVector>(2);
		typename TTraits::PairVector pairs{
			std::make_pair("Success", ValidationResult::Success),
			std::make_pair("Neutral", ValidationResult::Neutral),
		};

		auto pValidator = test::CreateValidator(pairs, states);
		auto dispatcher = TTraits::CreateDispatcher();

		// Act:
		decltype(test::CreateEntityInfos(0)) entityContainers[] = {
			test::CreateEntityInfos(1),
			test::CreateEntityInfos(2),
			test::CreateEntityInfos(4)
		};
		for (const auto& entities : entityContainers)
			TTraits::Dispatch(*pValidator, dispatcher, entities.toVector());

		// Assert:
		EXPECT_EQ(2u, test::GetNumSubValidators(*pValidator));
		EXPECT_EQ(7u, test::CounterAt(states, 0));
		EXPECT_EQ(7u, test::CounterAt(states, 1));
	}

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(NeutralResultDominatesSuccessResult) {
		// Arrange:
		auto states = test::CreateCounterStates<typename TTraits::ValidatorStateVector>(3);
		typename TTraits::PairVector pairs{
			std::make_pair("Success", ValidationResult::Success),
			std::make_pair("Neutral", ValidationResult::Neutral),
			std::make_pair("Success", ValidationResult::Success)
		};

		auto pValidator = test::CreateValidator(pairs, states);
		auto dispatcher = TTraits::CreateDispatcher();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		auto result = TTraits::Dispatch(*pValidator, dispatcher, entityInfos.toVector());

		// Assert:
		EXPECT_EQ(3u, test::GetNumSubValidators(*pValidator));
		EXPECT_EQ(1u, test::CounterAt(states, 0));
		EXPECT_EQ(1u, test::CounterAt(states, 1));
		EXPECT_EQ(1u, test::CounterAt(states, 2));
		EXPECT_EQ(ValidationResult::Neutral, result);
	}

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(FailureResultDominatesOtherResults) {
		// Arrange:
		auto states = test::CreateCounterStates<typename TTraits::ValidatorStateVector>(3);
		typename TTraits::PairVector pairs{
			std::make_pair("Success", ValidationResult::Success),
			std::make_pair("Neutral", ValidationResult::Neutral),
			std::make_pair("Failure", ValidationResult::Failure)
		};

		auto pValidator = test::CreateValidator(pairs, states);
		auto dispatcher = TTraits::CreateDispatcher();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		auto result = TTraits::Dispatch(*pValidator, dispatcher, entityInfos.toVector());

		// Assert:
		EXPECT_EQ(3u, test::GetNumSubValidators(*pValidator));
		EXPECT_EQ(1u, test::CounterAt(states, 0));
		EXPECT_EQ(1u, test::CounterAt(states, 1));
		EXPECT_EQ(1u, test::CounterAt(states, 2));
		EXPECT_EQ(ValidationResult::Failure, result);
	}

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(FailureShortCircuitsSubsequentValidations) {
		// Arrange:
		auto states = test::CreateCounterStates<typename TTraits::ValidatorStateVector>(3);
		typename TTraits::PairVector pairs{
			std::make_pair("Failure", ValidationResult::Failure),
			std::make_pair("Success", ValidationResult::Success),
			std::make_pair("Neutral", ValidationResult::Neutral)
		};

		auto pValidator = test::CreateValidator(pairs, states);
		auto dispatcher = TTraits::CreateDispatcher();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		auto result = TTraits::Dispatch(*pValidator, dispatcher, entityInfos.toVector());

		// Assert:
		EXPECT_EQ(3u, test::GetNumSubValidators(*pValidator));
		EXPECT_EQ(1u, test::CounterAt(states, 0));
		EXPECT_EQ(0u, test::CounterAt(states, 1));
		EXPECT_EQ(0u, test::CounterAt(states, 2));
		EXPECT_EQ(ValidationResult::Failure, result);
	}

	namespace {
		class MockPassThroughValidator : public stateful::EntityValidator {
		public:
			MockPassThroughValidator(size_t numEntities, const std::string& name)
					: m_entityInfos(numEntities)
					, m_contextPointers(numEntities)
					, m_name(name)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			ValidationResult validate(
					const model::WeakEntityInfo& entityInfo,
					const ValidatorContext& context) const override {
				// preallocate vectors in ctor and use Timestamp as index in order to prevent parallel race conditions
				auto index = test::GetTag(entityInfo.entity()).unwrap();
				m_entityInfos[index] = entityInfo;
				m_contextPointers[index] = &context;
				return ValidationResult::Success;
			}

		public:
			const auto& entityInfos() const {
				return m_entityInfos;
			}

			const auto& contextPointers() const {
				return m_contextPointers;
			}

		private:
			mutable std::vector<model::WeakEntityInfo> m_entityInfos;
			mutable std::vector<const ValidatorContext*> m_contextPointers;
			std::string m_name;
		};

		std::vector<const MockPassThroughValidator*> AddSubValidators(
				validators::ValidatorVectorT<const validators::ValidatorContext&>& builder,
				size_t numValidators,
				size_t numEntities) {
			std::vector<const MockPassThroughValidator*> validators;
			for (auto i = 0u; i < numValidators; ++i) {
				auto pMockValidator = std::make_unique<MockPassThroughValidator>(numEntities, std::to_string(i));
				validators.push_back(pMockValidator.get());
				builder.push_back(std::move(pMockValidator));
			}

			return validators;
		}
	}

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(AssertEntityInfosAreForwardedToChildValidators) {
		// Arrange:
		constexpr auto Num_Entities = 8u;
		cache::CatapultCache cache({});
		auto cacheView = cache.createView();
		auto context = test::CreateValidatorContext(Height(123), cacheView.toReadOnly());

		// - create entity infos
		auto sourceEntityInfos = test::CreateEntityInfos(Num_Entities);

		// - create an aggregate with five validators
		auto dispatcher = TTraits::CreateDispatcher();
		validators::ValidatorVectorT<const validators::ValidatorContext&> builder;
		auto validators = AddSubValidators(builder, 5, Num_Entities);
		auto pAggregateValidator = test::CreateAggregateValidator(std::move(builder));

		// Act:
		TTraits::Dispatch(*pAggregateValidator, dispatcher, sourceEntityInfos.toVector(), context);

		// Assert:
		auto i = 0u;
		for (const auto& pValidator : validators) {
			const auto& entityInfos = pValidator->entityInfos();
			const auto message = "validator at " + std::to_string(i);

			ASSERT_EQ(Num_Entities, entityInfos.size()) << message;
			EXPECT_EQ(sourceEntityInfos.toVector(), entityInfos) << message;
			++i;
		}
	}

	AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(AssertContextsAreForwardedToChildValidators) {
		// Arrange:
		constexpr auto Num_Entities = 8u;
		cache::CatapultCache cache({});
		auto cacheView = cache.createView();
		auto context = test::CreateValidatorContext(Height(123), cacheView.toReadOnly());

		// - create entity infos
		auto sourceEntityInfos = test::CreateEntityInfos(Num_Entities);

		// - create an aggregate with five validators
		auto dispatcher = TTraits::CreateDispatcher();
		validators::ValidatorVectorT<const validators::ValidatorContext&> builder;
		auto validators = AddSubValidators(builder, 5, Num_Entities);
		auto pAggregateValidator = test::CreateAggregateValidator(std::move(builder));

		// Act:
		TTraits::Dispatch(*pAggregateValidator, dispatcher, sourceEntityInfos.toVector(), context);

		// Assert:
		auto i = 0u;
		for (const auto& pValidator : validators) {
			const auto& contextPointers = pValidator->contextPointers();
			const auto message = "validator at " + std::to_string(i);

			ASSERT_EQ(Num_Entities, contextPointers.size()) << message;
			for (auto j = 0u; j < contextPointers.size(); ++j)
				EXPECT_EQ(&context, contextPointers[j]) << message << " context at " << j;

			++i;
		}
	}
}}
