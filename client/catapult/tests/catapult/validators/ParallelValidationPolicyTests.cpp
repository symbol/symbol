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
			PoolValidationPolicyPair(
					std::unique_ptr<thread::IoThreadPool>&& pPool,
					const std::shared_ptr<const StatelessEntityValidator>& pValidator)
					: m_pPool(std::move(pPool))
					, m_pValidationPolicy(CreateParallelValidationPolicy(*m_pPool, pValidator))
					, m_isReleased(false)
			{}

			~PoolValidationPolicyPair() {
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
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			std::shared_ptr<const ParallelValidationPolicy> m_pValidationPolicy;
			bool m_isReleased;
		};

		// endregion

		// region MockStatelessEntityValidator

		struct MultiThreadedValidatorStateValidatorTraits {
			using ItemType = model::VerifiableEntity;

			static uint64_t GetValue(const model::VerifiableEntity& entity) {
				// Deadline is set in GenerateBlockWithTransactions and used as a unique entity id
				return static_cast<const model::Transaction&>(entity).Deadline.unwrap();
			}
		};

		class MultiThreadedValidatorState : public test::BasicMultiThreadedState<MultiThreadedValidatorStateValidatorTraits> {
		public:
			void increment(const model::VerifiableEntity& entity) {
				process(entity);
			}
		};

		class MockStatelessEntityValidator : public StatelessEntityValidator {
		public:
			MockStatelessEntityValidator(const std::vector<ValidationResult>& results, std::atomic_bool* pWait)
					: m_results(results)
					, m_pWait(pWait)
					, m_name("MockStatelessEntityValidator")
					, m_counter(0)
			{}

		public:
			size_t numValidateCalls() const {
				return m_counter;
			}

			const auto& state() const {
				return m_state;
			}

		public:
			const std::string& name() const override {
				return m_name;
			}

			ValidationResult validate(const model::WeakEntityInfo& entityInfo) const override {
				// increment counter prior to wait in order for ValidateMany tests to work
				auto index = m_counter++;
				m_state.increment(entityInfo.entity());

				if (m_pWait)
					WAIT_FOR_EXPR(!*m_pWait);

				return m_results[std::min(index, m_results.size() - 1)];
			}

		private:
			std::vector<ValidationResult> m_results;
			std::atomic_bool* m_pWait;
			std::string m_name;
			mutable std::atomic<size_t> m_counter;
			mutable MultiThreadedValidatorState m_state;
		};

		// endregion

		auto CreateValidator(const std::vector<ValidationResult>& results, std::atomic_bool* pWait = nullptr) {
			return std::make_shared<MockStatelessEntityValidator>(results, pWait);
		}

		auto CreatePolicy(const std::shared_ptr<const StatelessEntityValidator>& pValidator, uint32_t numThreads = 0) {
			auto pPool = numThreads > 0 ? test::CreateStartedIoThreadPool(numThreads) : test::CreateStartedIoThreadPool();
			return PoolValidationPolicyPair(std::move(pPool), pValidator);
		}
	}

	// region traits

	namespace {
		struct ShortCircuitTraits {
			static auto Validate(const ParallelValidationPolicy& policy, const model::WeakEntityInfos& entityInfos) {
				return policy.validateShortCircuit(entityInfos);
			}

			static bool IsSuccess(ValidationResult result) {
				return IsValidationResultSuccess(result);
			}
		};

		struct AllTraits {
			static auto Validate(const ParallelValidationPolicy& policy, const model::WeakEntityInfos& entityInfos) {
				return policy.validateAll(entityInfos);
			}

			static bool IsSuccess(const std::vector<ValidationResult>& results) {
				return std::all_of(results.cbegin(), results.cend(), [](auto result) { return IsValidationResultSuccess(result); });
			}
		};
	}

#define PARALLEL_POLICY_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ShortCircuit) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ShortCircuitTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_All) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AllTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region basic

	PARALLEL_POLICY_TEST(ValidateInvokesValidateOnEachEntity) {
		// Arrange:
		auto pValidator = CreateValidator({ ValidationResult::Success });
		auto pPolicy = CreatePolicy(pValidator);

		// Act:
		auto entityInfos = test::CreateEntityInfos(3);
		TTraits::Validate(*pPolicy, entityInfos.toVector()).get();

		// Assert:
		EXPECT_EQ(3u, pValidator->numValidateCalls());
	}

	PARALLEL_POLICY_TEST(CanCallValidateMultipleTimesConsecutively) {
		// Arrange:
		auto pValidator = CreateValidator({ ValidationResult::Success });
		auto pPolicy = CreatePolicy(pValidator);

		// Act:
		for (const auto& entityInfos : { test::CreateEntityInfos(1), test::CreateEntityInfos(2), test::CreateEntityInfos(4) })
			TTraits::Validate(*pPolicy, entityInfos.toVector()).get();

		// Assert:
		EXPECT_EQ(7u, pValidator->numValidateCalls());
	}

	// endregion

	// region result precedence

	TEST(TEST_CLASS, NeutralResultDominatesSuccessResult_ShortCircuit) {
		// Arrange:
		auto pValidator = CreateValidator({ ValidationResult::Success, ValidationResult::Neutral, ValidationResult::Success });
		auto pPolicy = CreatePolicy(pValidator);

		// Act:
		auto entityInfos = test::CreateEntityInfos(3);
		auto result = ShortCircuitTraits::Validate(*pPolicy, entityInfos.toVector()).get();

		// Assert:
		EXPECT_EQ(3u, pValidator->numValidateCalls());
		EXPECT_EQ(ValidationResult::Neutral, result);
	}

	TEST(TEST_CLASS, FailureResultDominatesOtherResults_ShortCircuit) {
		// Arrange:
		auto pValidator = CreateValidator({ ValidationResult::Success, ValidationResult::Neutral, ValidationResult::Failure });
		auto pPolicy = CreatePolicy(pValidator);

		// Act:
		auto entityInfos = test::CreateEntityInfos(3);
		auto result = ShortCircuitTraits::Validate(*pPolicy, entityInfos.toVector()).get();

		// Assert:
		EXPECT_EQ(3u, pValidator->numValidateCalls());
		EXPECT_EQ(ValidationResult::Failure, result);
	}

	// endregion

	// region short-circuiting

	namespace {
		std::vector<ValidationResult> CreateAlternatingResults() {
			return {
				ValidationResult::Success,
				ValidationResult::Failure,
				ValidationResult::Neutral,
				ValidationResult::Failure,
				ValidationResult::Success
			};
		}
	}

	TEST(TEST_CLASS, FailureShortCircuitsSubsequentValidationsForSubsequentEntities_ShortCircuit) {
		// Arrange:
		auto pValidator = CreateValidator(CreateAlternatingResults());
		auto pPolicy = CreatePolicy(pValidator, 1);

		// Act:
		auto entityInfos = test::CreateEntityInfos(5);
		auto result = ShortCircuitTraits::Validate(*pPolicy, entityInfos.toVector()).get();

		// Assert: notice that the first entity that fails validation short circuits validation of all subsequent entities
		EXPECT_EQ(2u, pValidator->numValidateCalls());
		EXPECT_EQ(ValidationResult::Failure, result);
	}

	TEST(TEST_CLASS, FailureDoesNotShortCircuitSubsequentValidationsForSubsequentEntities_All) {
		// Arrange:
		auto pValidator = CreateValidator(CreateAlternatingResults());
		auto pPolicy = CreatePolicy(pValidator, 1);

		// Act:
		auto entityInfos = test::CreateEntityInfos(5);
		auto results = AllTraits::Validate(*pPolicy, entityInfos.toVector()).get();

		// Assert: notice that an independent result for each entity is returned
		EXPECT_EQ(5u, pValidator->numValidateCalls());
		EXPECT_EQ(CreateAlternatingResults(), results);
	}

	// endregion

	// region forwarding to validators

	namespace {
		class MockEntityCapturingStatelessEntityValidator : public StatelessEntityValidator {
		public:
			explicit MockEntityCapturingStatelessEntityValidator(size_t numEntities)
					: m_entityInfos(numEntities)
					, m_name("MockEntityCapturingStatelessEntityValidator")
			{}

		public:
			const auto& entityInfos() const {
				return m_entityInfos;
			}

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

		private:
			mutable std::vector<model::WeakEntityInfo> m_entityInfos;
			std::string m_name;
		};
	}

	PARALLEL_POLICY_TEST(EntityInfosAreForwardedToValidator) {
		// Arrange:
		auto pValidator = std::make_shared<MockEntityCapturingStatelessEntityValidator>(8);
		auto pPolicy = CreatePolicy(pValidator);

		auto sourceEntityInfos = test::CreateEntityInfos(8);

		// Act:
		TTraits::Validate(*pPolicy, sourceEntityInfos.toVector()).get();

		// Assert:
		const auto& entityInfos = pValidator->entityInfos();
		EXPECT_EQ(8u, entityInfos.size());
		EXPECT_EQ(sourceEntityInfos.toVector(), entityInfos);
	}

	// endregion

	// region multithreading

	PARALLEL_POLICY_TEST(FutureIsFulfilledEvenWhenValidatorIsDestroyed) {
		// Arrange:
		std::atomic_bool shouldBlock(true);
		auto pValidator = CreateValidator({ ValidationResult::Success }, &shouldBlock);
		auto pPolicy = CreatePolicy(pValidator);

		// Act: start a validate operation and then destroy the validator
		auto entityInfos = test::CreateEntityInfos(5);
		auto future = TTraits::Validate(*pPolicy, entityInfos.toVector());
		pPolicy.releaseValidationPolicy();

		// Assert: the validation should still complete successfully
		EXPECT_FALSE(future.is_ready());
		shouldBlock = false;

		// - wait for the future to complete
		future.get();
		EXPECT_EQ(5u, pValidator->numValidateCalls());
	}

	PARALLEL_POLICY_TEST(CanCallValidateMultipleTimesConcurrently) {
		// Arrange:
		auto pValidator = CreateValidator({ ValidationResult::Success });
		auto pPolicy = CreatePolicy(pValidator);

		// Act: compose futures to bool so that they are the same type in all template instantiations
		std::vector<thread::future<bool>> futures;
		std::vector<test::EntityInfoContainerWrapper> entityInfoGroups{
			test::CreateEntityInfos(1), test::CreateEntityInfos(2), test::CreateEntityInfos(4)
		};
		for (const auto& entityInfos : entityInfoGroups)
			futures.push_back(TTraits::Validate(*pPolicy, entityInfos.toVector()).then([](const auto&) { return true; }));

		// - wait for all futures
		std::for_each(futures.rbegin(), futures.rend(), [](auto& future) { future.get(); });

		// Assert:
		EXPECT_EQ(7u, pValidator->numValidateCalls());
	}

	// endregion

	// region work distribution

	namespace {
		const auto Num_Default_Threads = test::GetNumDefaultPoolThreads();

		template<typename TTraits, typename TAssertState>
		void ValidateMany(size_t numEntities, TAssertState assertState) {
			// Arrange:
			if (0 == numEntities) {
				// (Num_Default_Threads is 2 * cores, so Num_Default_Threads / 4 is nonzero when there are at least two cores)
				CATAPULT_LOG(fatal) << "Skipping test on single core system";
				return;
			}

			CATAPULT_LOG(debug) << "Running test with " << numEntities << " entities";

			// - create a validator
			std::atomic_bool shouldBlock(true);
			auto pValidator = CreateValidator({ ValidationResult::Success }, &shouldBlock);
			auto pPolicy = CreatePolicy(pValidator);

			// Act:
			auto entityInfos = test::CreateEntityInfos(numEntities);
			auto resultFuture = TTraits::Validate(*pPolicy, entityInfos.toVector());

			// - wait until every expected thread has incremented the counter once
			WAIT_FOR_EXPR(Num_Default_Threads <= pValidator->numValidateCalls());
			shouldBlock = false;

			auto result = resultFuture.get();

			// Assert:
			EXPECT_TRUE(TTraits::IsSuccess(result));
			assertState(pValidator->state());
		}

		template<typename TTraits>
		void AssertCanHandleManyValidatorsAndEntities(size_t numEntities) {
			// Act:
			ValidateMany<TTraits>(numEntities, [numEntities](const auto& state) {
				// Assert: validator was called numEntities times (with a unique entity)
				EXPECT_EQ(numEntities, state.counter());
				EXPECT_EQ(numEntities, state.numUniqueItems());
			});
		}

		template<typename TTraits>
		void AssertCanDistributeWorkEvenly(size_t numEntities) {
			// Act:
			ValidateMany<TTraits>(numEntities, [numEntities](const auto& state) {
				// Assert: validator was called numEntities times (with a unique entity)
				auto minWorkPerThread = numEntities / Num_Default_Threads;
				EXPECT_EQ(numEntities, state.counter());
				EXPECT_EQ(numEntities, state.numUniqueItems());

				// - the work was distributed evenly across threads
				//   (a thread can do more than the min amount of work if the number of entities is not divisible by
				//    the number of threads)
				for (auto counter : state.threadCounters())
					EXPECT_LE(minWorkPerThread, counter);

				EXPECT_EQ(Num_Default_Threads, state.threadCounters().size());
				EXPECT_EQ(Num_Default_Threads, state.sortedAndReducedThreadIds().size());
			});
		}
	}

	PARALLEL_POLICY_TEST(CanHandleManyValidatorsAndEntitiesWhenEntitiesAreMultipleOfThreads) {
		AssertCanHandleManyValidatorsAndEntities<TTraits>(Num_Default_Threads * 20);
	}

	PARALLEL_POLICY_TEST(CanHandleManyValidatorsAndEntitiesWhenEntitiesAreNotMultipleOfThreads) {
		AssertCanHandleManyValidatorsAndEntities<TTraits>(Num_Default_Threads / 4 * 81);
	}

	PARALLEL_POLICY_TEST(CanDistributeWorkEvenlyWhenEntitiesAreMultipleOfThreads) {
		AssertCanDistributeWorkEvenly<TTraits>(Num_Default_Threads * 20);
	}

	PARALLEL_POLICY_TEST(CanDistributeWorkEvenlyWhenEntitiesAreNotMultipleOfThreads) {
		AssertCanDistributeWorkEvenly<TTraits>(Num_Default_Threads / 4 * 81);
	}

	// endregion
}}
