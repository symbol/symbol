#include "ParallelValidationPolicy.h"
#include "AggregateValidationResult.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/ParallelFor.h"
#include "catapult/utils/Logging.h"
#include <boost/asio/io_service.hpp>
#include <algorithm>

namespace catapult { namespace validators {

	namespace {
		// region ShortCircuitTraits

		struct ShortCircuitTraits {
		public:
			using ResultType = ValidationResult;

			static constexpr bool IsEntityShortCircuitAllowed = true;

		public:
			explicit ShortCircuitTraits(size_t) : m_aggregateResult(ValidationResult::Success)
			{}

		public:
			bool validateEntity(const model::WeakEntityInfo& entityInfo, const ValidationFunction& validationFunction, size_t) {
				if (IsValidationResultFailure(m_aggregateResult))
					return false;

				auto result = validationFunction(entityInfo);
				AggregateValidationResult(m_aggregateResult, result);
				return true;
			}

			ValidationResult result() {
				return m_aggregateResult;
			}

		private:
			std::atomic<ValidationResult> m_aggregateResult;
		};

		// endregion

		// region AllTraits

		struct AllTraits {
		public:
			using ResultType = std::vector<ValidationResult>;

			static constexpr bool IsEntityShortCircuitAllowed = false;

		public:
			explicit AllTraits(size_t numEntities) : m_results(numEntities, ValidationResult::Success)
			{}

		public:
			bool validateEntity(const model::WeakEntityInfo& entityInfo, const ValidationFunction& validationFunction, size_t index) {
				auto result = validationFunction(entityInfo);
				AggregateValidationResult(m_results[index], result);
				return !IsValidationResultFailure(m_results[index]);
			}

			std::vector<ValidationResult> result() {
				return std::move(m_results);
			}

		private:
			std::vector<ValidationResult> m_results;
		};

		// endregion

		template<typename TTraits>
		class ValidationWork {
		public:
			ValidationWork(
					const std::shared_ptr<const void>& pOwner,
					const ValidationFunctions& validationFunctions,
					const model::WeakEntityInfos& entityInfos)
					: m_pOwner(pOwner) // extend the owner lifetime to the lifetime of this context
					, m_validationFunctions(validationFunctions)
					, m_entityInfos(entityInfos)
					, m_impl(m_entityInfos.size())
			{}

		public:
			const auto& entityInfos() const {
				return m_entityInfos;
			}

			auto future() {
				return m_promise.get_future();
			}

		public:
			void complete() {
				m_promise.set_value(std::move(m_impl.result()));
			}

			bool validateEntity(const model::WeakEntityInfo& entityInfo, size_t index) {
				for (const auto& validationFunction : m_validationFunctions) {
					if (m_impl.validateEntity(entityInfo, validationFunction, index))
						continue;

					// if allowed by policy, bypass processing of subsequent entities after failure
					if (TTraits::IsEntityShortCircuitAllowed)
						return false;

					// subsequent validators can always be bypassed after failure
					break;
				}

				return true;
			}

		private:
			std::shared_ptr<const void> m_pOwner;
			ValidationFunctions m_validationFunctions;
			model::WeakEntityInfos m_entityInfos;
			thread::promise<typename TTraits::ResultType> m_promise;
			TTraits m_impl;
		};

		class DefaultParallelValidationPolicy final
				: public ParallelValidationPolicy
				, public std::enable_shared_from_this<DefaultParallelValidationPolicy> {
		public:
			explicit DefaultParallelValidationPolicy(const std::shared_ptr<thread::IoServiceThreadPool>& pPool)
					: m_pPool(pPool)
					, m_service(pPool->service()) {
				CATAPULT_LOG(trace) << "DefaultParallelValidationPolicy created with " << pPool->numWorkerThreads() << " worker threads";
			}

		private:
			template<typename TTraits>
			auto validateT(const model::WeakEntityInfos& entityInfos, const ValidationFunctions& validationFunctions) const {
				auto pWork = std::make_shared<ValidationWork<TTraits>>(shared_from_this(), validationFunctions, entityInfos);
				return thread::compose(
						thread::ParallelFor(m_service, pWork->entityInfos(), m_pPool->numWorkerThreads(), [pWork](
								const auto& entityInfo,
								auto index) {
							return pWork->validateEntity(entityInfo, index);
						}),
						[pWork](const auto&) {
							pWork->complete();
							return pWork->future();
						});
			}

		public:
			thread::future<ValidationResult> validateShortCircuit(
					const model::WeakEntityInfos& entityInfos,
					const ValidationFunctions& validationFunctions) const override {
				return validateT<ShortCircuitTraits>(entityInfos, validationFunctions);
			}

			thread::future<std::vector<ValidationResult>> validateAll(
					const model::WeakEntityInfos& entityInfos,
					const ValidationFunctions& validationFunctions) const override {
				return validateT<AllTraits>(entityInfos, validationFunctions);
			}

		private:
			std::shared_ptr<const thread::IoServiceThreadPool> m_pPool;
			boost::asio::io_service& m_service;
		};
	}

	std::shared_ptr<const ParallelValidationPolicy> CreateParallelValidationPolicy(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
		return std::make_shared<const DefaultParallelValidationPolicy>(pPool);
	}
}}
