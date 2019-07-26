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

#include "catapult/validators/ParallelValidationPolicy.h"
#include "tests/catapult/validators/test/ValidationPolicyTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/nodeps/BasicMultiThreadedState.h"

namespace catapult { namespace validators {

#define TEST_CLASS ParallelValidationPolicyTests

	namespace {
		// region PoolValidationPolicyPair

		class PoolValidationPolicyPair {
		public:
			explicit PoolValidationPolicyPair(const std::shared_ptr<thread::IoThreadPool>& pPool)
					: m_pPool(pPool)
					, m_pValidationPolicy(CreateParallelValidationPolicy(m_pPool))
					, m_isReleased(false)
			{}

			~PoolValidationPolicyPair() {
				if (m_pPool)
					stopAll();
			}

		public:
			void stopAll() {
				// shutdown order is important
				// 1. wait for all validation operations to finish so that the only pointer is m_pValidationPolicy
				// 2. m_pPool->join waits for threads to complete but must finish before m_pValidationPolicy
				//    is destroyed
				if (!m_isReleased)
					test::WaitForUnique(m_pValidationPolicy, "m_pValidationPolicy");

				m_pPool->join();
			}

			void releaseValidationPolicy() {
				m_pValidationPolicy.reset();
				m_isReleased = true;
			}

		public:
			const ParallelValidationPolicy& operator*() {
				return *m_pValidationPolicy;
			}

		private:
			std::shared_ptr<thread::IoThreadPool> m_pPool;
			std::shared_ptr<const ParallelValidationPolicy> m_pValidationPolicy;
			bool m_isReleased;
		};

		// endregion

		// region traits

		struct ShortCircuitTraits {
			static auto Validate(
					const ParallelValidationPolicy& policy,
					const model::WeakEntityInfos& entityInfos,
					const ValidationFunctions& validationFunctions) {
				return policy.validateShortCircuit(entityInfos, validationFunctions);
			}

			static bool IsSuccess(ValidationResult result) {
				return IsValidationResultSuccess(result);
			}

			static ValidationResult GetFirstResult(ValidationResult result) {
				return result;
			}
		};

		struct AllTraits {
			static auto Validate(
					const ParallelValidationPolicy& policy,
					const model::WeakEntityInfos& entityInfos,
					const ValidationFunctions& validationFunctions) {
				return policy.validateAll(entityInfos, validationFunctions);
			}

			static bool IsSuccess(const std::vector<ValidationResult>& results) {
				return std::all_of(results.cbegin(), results.cend(), [](auto result) { return IsValidationResultSuccess(result); });
			}

			static ValidationResult GetFirstResult(const std::vector<ValidationResult>& results) {
				return results[0];
			}
		};

		// endregion
	}

#define PARALLEL_POLICY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ShortCircuit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ShortCircuitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_All) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AllTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
}}

namespace catapult { namespace validators {

	namespace {
		struct Counters {
		public:
			void resize(size_t count) {
				for (auto i = m_counters.size(); i < count; ++i)
					m_counters.push_back(std::make_unique<std::atomic<size_t>>(0));
			}

		public:
			std::atomic<size_t>& operator[](size_t index) {
				return *m_counters[index];
			}

		public:
			std::vector<size_t> toVector() const {
				std::vector<size_t> values;
				for (const auto& pCounter : m_counters)
					values.push_back(*pCounter);

				return values;
			}

		private:
			std::vector<std::unique_ptr<std::atomic<size_t>>> m_counters;
		};

		auto CreatePolicy(uint32_t numThreads = 0) {
			auto pPool = numThreads > 0 ? test::CreateStartedIoThreadPool(numThreads) : test::CreateStartedIoThreadPool();
			return PoolValidationPolicyPair(std::move(pPool));
		}

		ValidationFunctions CreateValidationFuncs(const std::vector<ValidationResult>& results, Counters& counters) {
			counters.resize(results.size());

			ValidationFunctions funcs;
			for (auto i = 0u; i < results.size(); ++i) {
				funcs.push_back([result = results[i], &counter = counters[i]](const auto&) {
					++counter;
					return result;
				});
			}

			return funcs;
		}

		ValidationFunctions CreateValidationFuncs(
				const std::vector<ValidationResult>& results,
				Counters& counters,
				std::atomic_bool& wait) {
			counters.resize(results.size());

			ValidationFunctions funcs;
			for (auto i = 0u; i < results.size(); ++i) {
				funcs.push_back([result = results[i], &counter = counters[i], &wait](const auto& entityInfo) {
					// the thread that handles the first entity signals other threads when to continue
					if (0 == entityInfo.hash()[0]) {
						if (IsValidationResultFailure(result))
							wait = false;
					} else {
						WAIT_FOR_EXPR(!wait);
						test::Sleep(1); // give the first thread a chance to update the aggregate result
					}

					++counter;
					return result;
				});
			}

			return funcs;
		}
	}

	// region basic

	PARALLEL_POLICY_TEST(ValidateInvokesValidateOnEachContainedValidator) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Neutral }, counters);
		auto pPolicy = CreatePolicy();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get();

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 1, 1 }), counters.toVector());
	}

	PARALLEL_POLICY_TEST(ValidateInvokesValidateOnEachEntity) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success }, counters);
		auto pPolicy = CreatePolicy();

		// Act:
		auto entityInfos = test::CreateEntityInfos(3);
		TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get();

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 3 }), counters.toVector());
	}

	PARALLEL_POLICY_TEST(CanCallValidateMultipleTimesConsecutively) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Neutral }, counters);
		auto pPolicy = CreatePolicy();

		// Act:
		for (const auto& entityInfos : { test::CreateEntityInfos(1), test::CreateEntityInfos(2), test::CreateEntityInfos(4) })
			TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get();

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 7, 7 }), counters.toVector());
	}

	// endregion

	// region result precedence

	PARALLEL_POLICY_TEST(NeutralResultDominatesSuccessResult) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Neutral, ValidationResult::Success }, counters);
		auto pPolicy = CreatePolicy();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		auto result = TTraits::GetFirstResult(TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get());

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 1, 1, 1 }), counters.toVector());
		EXPECT_EQ(ValidationResult::Neutral, result);
	}

	PARALLEL_POLICY_TEST(FailureResultDominatesOtherResults) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Neutral, ValidationResult::Failure }, counters);
		auto pPolicy = CreatePolicy();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		auto result = TTraits::GetFirstResult(TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get());

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 1, 1, 1 }), counters.toVector());
		EXPECT_EQ(ValidationResult::Failure, result);
	}

	// endregion

	// region short-circuiting

	PARALLEL_POLICY_TEST(FailureShortCircuitsSubsequentValidationsForSameEntity) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Failure, ValidationResult::Success, ValidationResult::Neutral }, counters);
		auto pPolicy = CreatePolicy();

		// Act:
		auto entityInfos = test::CreateEntityInfos(1);
		auto result = TTraits::GetFirstResult(TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get());

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 1, 0, 0 }), counters.toVector());
		EXPECT_EQ(ValidationResult::Failure, result);
	}

	TEST(TEST_CLASS, FailureShortCircuitsSubsequentValidationsForSubsequentEntities_ShortCircuit) {
		// Arrange:
		auto counters = Counters();
		std::atomic_bool wait(true);
		auto funcs = CreateValidationFuncs(
				{ ValidationResult::Success, ValidationResult::Failure, ValidationResult::Neutral },
				counters,
				wait);
		auto pPolicy = CreatePolicy();

		// Act:
		auto entityInfos = test::CreateEntityInfos(5);
		auto result = ShortCircuitTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get();

		// Assert: notice that the first entity that fails validation short circuits validation of all subsequent entities
		// - note also that we cannot have an exact assert on the first counter since all threads might already have called
		//   the first validation function before the validation result is set to failure
		EXPECT_LE(1u, counters[0]);
		EXPECT_EQ(1u, counters[1]);
		EXPECT_EQ(0u, counters[2]);
		EXPECT_EQ(ValidationResult::Failure, result);
	}

	TEST(TEST_CLASS, FailureDoesNotShortCircuitSubsequentValidationsForSubsequentEntitiesOnePerThread_All) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Failure, ValidationResult::Neutral }, counters);
		auto pPolicy = CreatePolicy();

		// Act:
		auto entityInfos = test::CreateEntityInfos(5);
		auto results = AllTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get();

		// Assert: notice that an independent result for each entity is returned
		EXPECT_EQ(std::vector<size_t>({ 5, 5, 0 }), counters.toVector());
		EXPECT_EQ(std::vector<ValidationResult>(5, ValidationResult::Failure), results);
	}

	TEST(TEST_CLASS, FailureDoesNotShortCircuitSubsequentValidationsForSubsequentEntitiesMultiplePerThread_All) {
		// Arrange: force the policy to use two threads, which will cause multiple entities to be processed by each thread
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Failure, ValidationResult::Neutral }, counters);
		auto pPolicy = CreatePolicy(2);

		// Act:
		auto entityInfos = test::CreateEntityInfos(5);
		auto results = AllTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get();

		// Assert: notice that an independent result for each entity is returned
		EXPECT_EQ(std::vector<size_t>({ 5, 5, 0 }), counters.toVector());
		EXPECT_EQ(std::vector<ValidationResult>(5, ValidationResult::Failure), results);
	}

	// endregion

	// region forwarding to validators

	namespace {
		class MockPassThroughValidator : public stateless::EntityValidator {
		public:
			MockPassThroughValidator(size_t numEntities, const std::string& name)
					: m_entityInfos(numEntities)
					, m_name(name)
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			ValidationResult validate(const model::WeakEntityInfo& entityInfo) const override {
				// preallocate vectors in ctor and use Deadline as index in order to prevent parallel race conditions
				auto index = entityInfo.cast<model::Transaction>().entity().Deadline.unwrap();
				m_entityInfos[index] = entityInfo;
				return ValidationResult::Success;
			}

		public:
			const auto& entityInfos() const {
				return m_entityInfos;
			}

		private:
			mutable std::vector<model::WeakEntityInfo> m_entityInfos;
			std::string m_name;
		};

		std::vector<const MockPassThroughValidator*> AddSubValidators(
				ValidationFunctions& funcs,
				size_t numValidators,
				size_t numEntities) {
			std::vector<const MockPassThroughValidator*> validators;
			for (auto i = 0u; i < numValidators; ++i) {
				auto pMockValidator = std::make_shared<MockPassThroughValidator>(numEntities, std::to_string(i));

				funcs.push_back([pMockValidator](const auto& entityInfo) {
					return pMockValidator->validate(entityInfo);
				});

				validators.push_back(pMockValidator.get());
			}

			return validators;
		}
	}

	PARALLEL_POLICY_TEST(AssertEntityInfosAreForwardedToChildValidators) {
		// Arrange: create entity infos
		constexpr auto Num_Entities = 8u;
		auto sourceEntityInfos = test::CreateEntityInfos(Num_Entities);

		// - create five validators and a policy
		ValidationFunctions funcs;
		auto validators = AddSubValidators(funcs, 5, Num_Entities);
		auto pPolicy = CreatePolicy();

		// Act:
		TTraits::Validate(*pPolicy, sourceEntityInfos.toVector(), funcs).get();

		// Assert:
		auto i = 0u;
		for (const auto& pValidator : validators) {
			const auto& entityInfos = pValidator->entityInfos();
			const auto message = "validator at " + std::to_string(i);

			EXPECT_EQ(Num_Entities, entityInfos.size()) << message;
			EXPECT_EQ(sourceEntityInfos.toVector(), entityInfos) << message;
			++i;
		}
	}

	// endregion

	// region multithreading

	PARALLEL_POLICY_TEST(FutureIsFulfilledEvenWhenValidatorIsDestroyed) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Neutral }, counters);
		auto pPolicy = CreatePolicy();

		std::atomic_bool shouldBlock(true);
		funcs.push_back([&shouldBlock](const auto&) {
			WAIT_FOR_EXPR(!shouldBlock);
			return ValidationResult::Success;
		});

		// Act: start a validate operation and then destroy the validator
		auto entityInfos = test::CreateEntityInfos(5);
		auto future = TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs);
		pPolicy.releaseValidationPolicy();

		// Assert: the validation should still complete successfully
		EXPECT_FALSE(future.is_ready());
		shouldBlock = false;

		// - wait for the future to complete
		future.get();
		EXPECT_EQ(std::vector<size_t>({ 5, 5 }), counters.toVector());
	}

	PARALLEL_POLICY_TEST(CanCallValidateMultipleTimesConcurrently) {
		// Arrange:
		auto counters = Counters();
		auto funcs = CreateValidationFuncs({ ValidationResult::Success, ValidationResult::Neutral }, counters);
		auto pPolicy = CreatePolicy();

		// Act: compose futures to bool so that they are the same type in all template instantiations
		std::vector<thread::future<bool>> futures;
		for (const auto& entityInfos : { test::CreateEntityInfos(1), test::CreateEntityInfos(2), test::CreateEntityInfos(4) })
			futures.push_back(TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).then([](const auto&) { return true; }));

		// - wait for all futures
		std::for_each(futures.rbegin(), futures.rend(), [](auto& future) { future.get(); });

		// Assert:
		EXPECT_EQ(std::vector<size_t>({ 7, 7 }), counters.toVector());
	}

	// endregion

	// region work distribution

	namespace {
		const auto Num_Default_Threads = test::GetNumDefaultPoolThreads();

		struct ValidatorTraits {
			using ItemType = model::VerifiableEntity;

			static uint64_t GetValue(const model::VerifiableEntity& entity) {
				// Deadline is set in GenerateBlockWithTransactions and used as a unique entity id
				return static_cast<const model::Transaction&>(entity).Deadline.unwrap();
			}
		};

		class MultiThreadedValidatorState : public test::BasicMultiThreadedState<ValidatorTraits> {
		public:
			void increment(const model::VerifiableEntity& entity) {
				process(entity);
			}
		};

		using MultiThreadedValidatorStates = std::vector<std::unique_ptr<MultiThreadedValidatorState>>;

		template<typename TTraits>
		void ValidateMany(MultiThreadedValidatorStates& states, size_t numValidators, size_t numEntities) {
			// Arrange:
			if (0 == numEntities) {
				// (Num_Default_Threads is 2 * cores, so Num_Default_Threads / 4 is nonzero when there are
				// at least two cores)
				CATAPULT_LOG(fatal) << "Skipping test on single core system";
				return;
			}

			CATAPULT_LOG(debug) << "Running test with " << numValidators << " validators and " << numEntities << " entities";

			// - create an aggregate of MockStatefulBlockingValidator
			std::atomic<size_t> counter(0);
			auto funcs = ValidationFunctions();
			for (auto i = 0u; i < numValidators; ++i) {
				funcs.push_back([&state = *states[i], &counter](const auto& entityInfo) {
					// - increment the counter and wait until every expected thread has incremented it once
					++counter;
					WAIT_FOR_EXPR(counter >= Num_Default_Threads);
					state.increment(entityInfo.entity());
					return ValidationResult::Success;
				});
			}

			auto pPolicy = CreatePolicy();

			// Act:
			auto entityInfos = test::CreateEntityInfos(numEntities);
			auto result = TTraits::Validate(*pPolicy, entityInfos.toVector(), funcs).get();

			// Assert:
			EXPECT_TRUE(TTraits::IsSuccess(result));
		}

		MultiThreadedValidatorStates CreateMultithreadedStates(size_t count) {
			MultiThreadedValidatorStates states;
			for (auto i = 0u; i < count; ++i)
				states.push_back(std::make_unique<MultiThreadedValidatorState>());

			return states;
		}

		template<typename TTraits>
		void AssertCanHandleManyValidatorsAndEntities(size_t numValidators, size_t numEntities) {
			// Arrange:
			auto states = CreateMultithreadedStates(numValidators);

			// Act:
			ValidateMany<TTraits>(states, numValidators, numEntities);

			// Assert: each validator was called numEntities times (with a unique entity)
			for (auto i = 0u; i < numValidators; ++i) {
				const auto& state = *states[i];
				EXPECT_EQ(numEntities, state.counter()) << "validator " << i;
				EXPECT_EQ(numEntities, state.numUniqueItems()) << "validator " << i;
			}
		}

		template<typename TTraits>
		void AssertCanDistributeWorkEvenly(size_t numValidators, size_t numEntities) {
			// Arrange:
			auto states = CreateMultithreadedStates(numValidators);

			// Act:
			ValidateMany<TTraits>(states, numValidators, numEntities);

			// Assert: each validator was called numEntities times (with a unique entity)
			auto minWorkPerThread = numEntities / Num_Default_Threads;
			for (auto i = 0u; i < numValidators; ++i) {
				const auto& state = *states[i];
				EXPECT_EQ(numEntities, state.counter()) << "validator " << i;
				EXPECT_EQ(numEntities, state.numUniqueItems()) << "validator " << i;

				// - the work was distributed evenly across threads
				//   (a thread can do more than the min amount of work if the number of entities is not divisible by
				//    the number of threads)
				for (auto counter : state.threadCounters())
					EXPECT_LE(minWorkPerThread, counter) << "validator " << i;

				EXPECT_EQ(Num_Default_Threads, state.threadCounters().size());
				EXPECT_EQ(Num_Default_Threads, state.sortedAndReducedThreadIds().size());
			}
		}
	}

	PARALLEL_POLICY_TEST(CanHandleManyValidatorsAndEntitiesWhenEntitiesAreMultipleOfThreads) {
		AssertCanHandleManyValidatorsAndEntities<TTraits>(100, Num_Default_Threads * 20);
	}

	PARALLEL_POLICY_TEST(CanHandleManyValidatorsAndEntitiesWhenEntitiesAreNotMultipleOfThreads) {
		AssertCanHandleManyValidatorsAndEntities<TTraits>(100, Num_Default_Threads / 4 * 81);
	}

	PARALLEL_POLICY_TEST(CanDistributeWorkEvenlyWhenEntitiesAreMultipleOfThreads) {
		AssertCanDistributeWorkEvenly<TTraits>(100, Num_Default_Threads * 20);
	}

	PARALLEL_POLICY_TEST(CanDistributeWorkEvenlyWhenEntitiesAreNotMultipleOfThreads) {
		AssertCanDistributeWorkEvenly<TTraits>(100, Num_Default_Threads / 4 * 81);
	}

	// endregion
}}
