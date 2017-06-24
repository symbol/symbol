#include "catapult/validators/ParallelValidationPolicy.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/catapult/validators/utils/MockEntityValidator.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/EntityTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/nodeps/BasicMultiThreadedState.h"
#include "tests/TestHarness.h"
#include <mutex>
#include <thread>

using namespace catapult::model;

namespace catapult { namespace validators {

#define TEST_CLASS ParallelValidationPolicyTests

	namespace {
		struct ValidatorTraits {
			using ItemType = model::VerifiableEntity;

			static uint64_t GetValue(const model::VerifiableEntity& entity) {
				// timestamp is set in GenerateRandomBlockWithTransactions and used as a unique entity id
				return test::GetTag(entity).unwrap();
			}
		};

		class MultiThreadedValidatorState : public test::BasicMultiThreadedState<ValidatorTraits> {
		public:
			void increment(const model::VerifiableEntity& entity) {
				process(entity);
			}
		};

		class PoolValidationPolicyPair {
		public:
			explicit PoolValidationPolicyPair(
					const std::shared_ptr<thread::IoServiceThreadPool>& pPool,
					const ParallelValidationPolicyFunc& executionPolicyFunc)
					: m_pPool(pPool)
					, m_executionPolicyFunc(executionPolicyFunc)
					, m_isReleased(false)
			{}

			~PoolValidationPolicyPair() {
				if (m_pPool) stopAll();
			}

		public:
			void stopAll() {
				// shutdown order is important
				// 1. wait for all validation operations to finish so that the only pointer is m_pValidationPolicy
				// 2. m_pPool->join waits for threads to complete but must finish before m_pValidationPolicy
				//    is destroyed
				if (!m_isReleased)
					test::WaitForUnique(m_executionPolicyFunc.owner(), "m_pValidationPolicy");

				m_pPool->join();
			}

			void releaseValidationPolicy() {
				m_executionPolicyFunc.reset();
				m_isReleased = true;
			}

		public:
			thread::future<ValidationResult> operator()(
					const WeakEntityInfos& entityInfos,
					const ValidationFunctions& validationFunctions) const {
				return m_executionPolicyFunc(entityInfos, validationFunctions);
			}

		private:
			std::shared_ptr<thread::IoServiceThreadPool> m_pPool;
			ParallelValidationPolicyFunc m_executionPolicyFunc;
			bool m_isReleased;

		public:
			PoolValidationPolicyPair(PoolValidationPolicyPair&& rhs) = default;
		};

		struct ParallelValidationPolicyTraits {
			using PairVector = std::vector<std::pair<std::string, ValidationResult>>;
			using ValidatorType = mocks::MockEntityValidator<MultiThreadedValidatorState>;
			using ValidatorStateVector = std::vector<std::shared_ptr<MultiThreadedValidatorState>>;

			static auto CreateDispatcher() {
				std::shared_ptr<thread::IoServiceThreadPool> pPool = test::CreateStartedIoServiceThreadPool();
				auto pPolicy = CreateParallelValidationPolicy(pPool);
				return PoolValidationPolicyPair(pPool, pPolicy);
			}

			template<typename TValidator, typename TEntityInfos>
			static ValidationResult Dispatch(
					const TValidator& validator,
					const PoolValidationPolicyPair& dispatcher,
					const TEntityInfos& entityInfos) {
				mocks::EmptyValidatorContext context;
				return validator.curry(context.cref()).dispatch(dispatcher, entityInfos).get();
			}

			template<typename TValidator>
			static ValidationResult Dispatch(
					const TValidator& validator,
					const PoolValidationPolicyPair& dispatcher,
					const model::WeakEntityInfos& entityInfos,
					const ValidatorContext& context) {
				return validator.curry(std::cref(context)).dispatch(dispatcher, entityInfos).get();
			}
		};
	}
}}

#define AGGREGATE_VALIDATOR_VALIDATION_POLICY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ParallelValidationPolicyTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#include "tests/catapult/validators/utils/ValidationPolicyTests.h"

namespace catapult { namespace validators {
	using Traits = ParallelValidationPolicyTraits;
	using PairVector = ParallelValidationPolicyTraits::PairVector;
	using ValidatorType = ParallelValidationPolicyTraits::ValidatorType;
	using ValidatorStateVector = ParallelValidationPolicyTraits::ValidatorStateVector;

	namespace {
		class BlockingValidator : public stateful::EntityValidator {
		public:
			explicit BlockingValidator(std::atomic_bool& block) : m_block(block), m_name("BlockingValidator")
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			ValidationResult validate(const model::WeakEntityInfo&, const ValidatorContext&) const override {
				WAIT_FOR_VALUE(m_block, false);
				return ValidationResult::Success;
			}

		private:
			std::atomic_bool& m_block;
			std::string m_name;
		};

		auto CreateBlockingValidator(std::atomic_bool& block) {
			return std::make_unique<BlockingValidator>(block);
		}
	}

	TEST(TEST_CLASS, FutureIsFulfilledEvenIfValidatorIsDestroyed) {
		// Arrange:
		auto states = test::CreateCounterStates<ValidatorStateVector>(2);
		PairVector pairs{
			std::make_pair("Success", ValidationResult::Success),
			std::make_pair("Neutral", ValidationResult::Neutral),
		};

		std::atomic_bool block(true);
		validators::ValidatorVectorT<const validators::ValidatorContext&> builder;
		test::AddValidators(builder, pairs, states);
		builder.push_back(CreateBlockingValidator(block));
		auto pValidator = test::CreateAggregateValidator(std::move(builder));
		auto dispatcher = Traits::CreateDispatcher();

		// Act: start a validate operation and then destroy the validator
		mocks::EmptyValidatorContext context;
		auto entityInfos = test::CreateEntityInfos(5);
		auto future = pValidator->curry(context.cref()).dispatch(dispatcher, entityInfos.toVector());
		dispatcher.releaseValidationPolicy();

		// Assert: the validation should still have completed successfully
		EXPECT_FALSE(future.is_ready());
		block = false;

		EXPECT_EQ(ValidationResult::Neutral, future.get());
		EXPECT_EQ(5u, test::CounterAt(states, 0));
		EXPECT_EQ(5u, test::CounterAt(states, 1));
	}

	TEST(TEST_CLASS, CanCallValidateMultipleTimesConcurrently) {
		// Arrange:
		auto states = test::CreateCounterStates<ValidatorStateVector>(2);
		PairVector pairs{
			std::make_pair("Success", ValidationResult::Success),
			std::make_pair("Neutral", ValidationResult::Neutral),
		};

		auto pValidator = test::CreateValidator(pairs, states);
		auto dispatcher = Traits::CreateDispatcher();

		// Act:
		mocks::EmptyValidatorContext context;
		decltype(test::CreateEntityInfos(0)) entityContainers[] = {
			test::CreateEntityInfos(1),
			test::CreateEntityInfos(2),
			test::CreateEntityInfos(4)
		};
		std::vector<thread::future<ValidationResult>> futures;
		for (const auto& entities : entityContainers)
			futures.push_back(pValidator->curry(context.cref()).dispatch(dispatcher, entities.toVector()));

		// - wait for all futures
		std::for_each(futures.rbegin(), futures.rend(), [](auto& future) { future.get(); });

		// Assert:
		EXPECT_EQ(2u, test::GetNumSubValidators(*pValidator));
		EXPECT_EQ(7u, test::CounterAt(states, 0));
		EXPECT_EQ(7u, test::CounterAt(states, 1));
	}

	namespace {
		const auto Num_Default_Threads = test::GetNumDefaultPoolThreads();

		class MockStatefulBlockingValidator : public mocks::MockEntityValidator<MultiThreadedValidatorState> {
		private:
			using Base = mocks::MockEntityValidator<MultiThreadedValidatorState>;

		public:
			MockStatefulBlockingValidator(
					const std::shared_ptr<MultiThreadedValidatorState>& pState,
					std::atomic<size_t>& counter,
					const std::string& name)
					: Base(ValidationResult::Success, pState, name)
					, m_counter(counter)
			{}

		public:
			ValidationResult validate(
					const model::WeakEntityInfo& entityInfo,
					const ValidatorContext& context) const override {
				// - increment the counter and wait until every expected thread has incremented it once
				++m_counter;
				WAIT_FOR_EXPR(m_counter >= Num_Default_Threads);
				return Base::validate(entityInfo, context);
			}

		private:
			std::atomic<size_t>& m_counter;
		};

		void ValidateMany(ValidatorStateVector& states, size_t numValidators, size_t numEntities) {
			// Arrange:
			if (0 == numEntities) {
				// (Num_Default_Threads is 2 * cores, so Num_Default_Threads / 4 is non zero when there are
				// at least two cores)
				CATAPULT_LOG(fatal) << "Skipping test on single core system";
				return;
			}

			CATAPULT_LOG(debug) << "Running test with " << numValidators << " validators and "
					<< numEntities << " entities";

			// - create an aggregate of MockStatefulBlockingValidator
			std::atomic<size_t> counter(0);
			validators::ValidatorVectorT<const validators::ValidatorContext&> builder;
			for (auto i = 0u; i < numValidators; ++i) {
				auto pValidator = std::make_unique<MockStatefulBlockingValidator>(states[i], counter, std::to_string(i));
				builder.push_back(std::move(pValidator));
			}

			auto pValidator = test::CreateAggregateValidator(std::move(builder));
			auto dispatcher = Traits::CreateDispatcher();

			// Act:
			auto entityInfos = test::CreateEntityInfos(numEntities);
			auto result = Traits::Dispatch(*pValidator, dispatcher, entityInfos.toVector());

			// Assert:
			// - the parallel validator was created around the expected number of validators and returned success
			EXPECT_EQ(numValidators, test::GetNumSubValidators(*pValidator));
			EXPECT_EQ(ValidationResult::Success, result);
		}

		void AssertCanHandleManyValidatorsAndEntities(size_t numValidators, size_t numEntities) {
			// Arrange:
			auto states = test::CreateCounterStates<ValidatorStateVector>(numValidators);

			// Act:
			ValidateMany(states, numValidators, numEntities);

			// Assert: each validator was called numEntities times (with a unique entity)
			for (auto i = 0u; i < numValidators; ++i) {
				auto pState = states[i];
				EXPECT_EQ(numEntities, pState->counter()) << "validator " << i;
				EXPECT_EQ(numEntities, pState->numUniqueItems()) << "validator " << i;
			}
		}

		void AssertCanDistributeWorkEvenly(size_t numValidators, size_t numEntities) {
			// Arrange:
			auto states = test::CreateCounterStates<ValidatorStateVector>(numValidators);

			// Act:
			ValidateMany(states, numValidators, numEntities);

			// Assert: each validator was called numEntities times (with a unique entity)
			auto minWorkPerThread = numEntities / Num_Default_Threads;
			for (auto i = 0u; i < numValidators; ++i) {
				auto pState = states[i];
				EXPECT_EQ(numEntities, pState->counter()) << "validator " << i;
				EXPECT_EQ(numEntities, pState->numUniqueItems()) << "validator " << i;

				// - the work was distributed evenly across threads
				//   (a thread can do more than the min amount of work if the number of entities is not divisible by
				//    the number of threads)
				for (auto counter : pState->threadCounters())
					EXPECT_LE(minWorkPerThread, counter) << "validator " << i;

				EXPECT_EQ(Num_Default_Threads, pState->threadCounters().size());
				EXPECT_EQ(Num_Default_Threads, pState->sortedAndReducedThreadIds().size());
			}
		}
	}

	TEST(TEST_CLASS, CanHandleManyValidatorsAndEntitiesWhenEntitiesAreMultipleOfThreads) {
		// Assert:
		AssertCanHandleManyValidatorsAndEntities(100, Num_Default_Threads * 20);
	}

	TEST(TEST_CLASS, CanHandleManyValidatorsAndEntitiesWhenEntitiesAreNotMultipleOfThreads) {
		// Assert:
		AssertCanHandleManyValidatorsAndEntities(100, Num_Default_Threads / 4 * 81);
	}

	TEST(TEST_CLASS, CanDistributeWorkEvenlyWhenEntitiesAreMultipleOfThreads) {
		// Assert:
		AssertCanDistributeWorkEvenly(100, Num_Default_Threads * 20);
	}

	TEST(TEST_CLASS, CanDistributeWorkEvenlyWhenEntitiesAreNotMultipleOfThreads) {
		// Assert:
		AssertCanDistributeWorkEvenly(100, Num_Default_Threads / 4 * 81);
	}
}}
