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
		class ValidationWork {
		public:
			ValidationWork(
					const std::shared_ptr<const void>& pOwner,
					const ValidationFunctions& validationFunctions,
					const model::WeakEntityInfos& entityInfos)
					: m_pOwner(pOwner) // extend the owner lifetime to the lifetime of this context
					, m_validationFunctions(validationFunctions)
					, m_entityInfos(entityInfos)
					, m_aggregateResult(ValidationResult::Success)
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
				m_promise.set_value(m_aggregateResult);
			}

			bool validateEntity(const model::WeakEntityInfo& entityInfo) {
				for (const auto& validationFunction : m_validationFunctions) {
					if (IsValidationResultFailure(m_aggregateResult))
						return false;

					auto result = validationFunction(entityInfo);
					AggregateValidationResult(m_aggregateResult, result);
				}

				return true;
			}

		private:
			std::shared_ptr<const void> m_pOwner;
			ValidationFunctions m_validationFunctions;
			model::WeakEntityInfos m_entityInfos;
			std::atomic<ValidationResult> m_aggregateResult;
			thread::promise<ValidationResult> m_promise;
		};

		class ParallelValidationPolicy final
				: public std::enable_shared_from_this<ParallelValidationPolicy> {
		public:
			explicit ParallelValidationPolicy(const std::shared_ptr<thread::IoServiceThreadPool>& pPool)
					: m_pPool(pPool)
					, m_service(pPool->service()) {
				CATAPULT_LOG(trace) << "DefaultParallelValidationPolicy created with "
					<< pPool->numWorkerThreads() << " worker threads";
			}

		public:
			thread::future<ValidationResult> operator()(
					const model::WeakEntityInfos& entityInfos,
					const ValidationFunctions& validationFunctions) const {
				auto pWork = std::make_shared<ValidationWork>(shared_from_this(), validationFunctions, entityInfos);
				return thread::compose(
						thread::ParallelFor(m_service, pWork->entityInfos(), m_pPool->numWorkerThreads(), [pWork](const auto& entityInfo) {
							return pWork->validateEntity(entityInfo);
						}),
						[pWork](const auto&) {
							pWork->complete();
							return pWork->future();
						});
			}

		private:
			std::shared_ptr<const thread::IoServiceThreadPool> m_pPool;
			boost::asio::io_service& m_service;
		};
	}

	ParallelValidationPolicyFunc CreateParallelValidationPolicy(
			const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
		auto pPolicy = std::make_shared<const ParallelValidationPolicy>(pPool);
		return ParallelValidationPolicyFunc([pPolicy = pPolicy.get()](const auto& entityInfos, const auto& validationFunctions) {
			return (*pPolicy)(entityInfos, validationFunctions);
		}, pPolicy);
	}
}}
