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

#include "ParallelValidationPolicy.h"
#include "AggregateValidationResult.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/thread/ParallelFor.h"
#include "catapult/utils/Logging.h"
#include <boost/asio/io_context.hpp>
#include <algorithm>

namespace catapult { namespace validators {

	namespace {
		// region ShortCircuitTraits

		struct ShortCircuitTraits {
		public:
			using ResultType = ValidationResult;

		public:
			explicit ShortCircuitTraits(size_t) : m_aggregateResult(ValidationResult::Success)
			{}

		public:
			bool validateEntity(const StatelessEntityValidator& validator, const model::WeakEntityInfo& entityInfo, size_t) {
				if (IsValidationResultFailure(m_aggregateResult))
					return false;

				auto result = validator.validate(entityInfo);
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

		public:
			explicit AllTraits(size_t numEntities) : m_results(numEntities, ValidationResult::Success)
			{}

		public:
			bool validateEntity(const StatelessEntityValidator& validator, const model::WeakEntityInfo& entityInfo, size_t index) {
				auto result = validator.validate(entityInfo);
				AggregateValidationResult(m_results[index], result);
				return true;
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
			ValidationWork(const std::shared_ptr<const StatelessEntityValidator>& pValidator, const model::WeakEntityInfos& entityInfos)
					: m_pValidator(pValidator)
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
				return m_impl.validateEntity(*m_pValidator, entityInfo, index);
			}

		private:
			std::shared_ptr<const StatelessEntityValidator> m_pValidator;
			model::WeakEntityInfos m_entityInfos;
			thread::promise<typename TTraits::ResultType> m_promise;
			TTraits m_impl;
		};

		class DefaultParallelValidationPolicy final : public ParallelValidationPolicy {
		public:
			DefaultParallelValidationPolicy(thread::IoThreadPool& pool, const std::shared_ptr<const StatelessEntityValidator>& pValidator)
					: m_pool(pool)
					, m_pValidator(pValidator) {
				CATAPULT_LOG(trace) << "DefaultParallelValidationPolicy created with " << m_pool.numWorkerThreads() << " worker threads";
			}

		private:
			template<typename TTraits>
			auto validateT(const model::WeakEntityInfos& entityInfos) const {
				auto pWork = std::make_shared<ValidationWork<TTraits>>(m_pValidator, entityInfos);

				auto workProcessItemCallback = [pWork](const auto& entityInfo, auto index) {
					return pWork->validateEntity(entityInfo, index);
				};
				auto workCompleteCallback = [pWork](const auto&) {
					pWork->complete();
					return pWork->future();
				};

				return thread::compose(
						thread::ParallelFor(m_pool.ioContext(), pWork->entityInfos(), m_pool.numWorkerThreads(), workProcessItemCallback),
						workCompleteCallback);
			}

		public:
			thread::future<ValidationResult> validateShortCircuit(const model::WeakEntityInfos& entityInfos) const override {
				return validateT<ShortCircuitTraits>(entityInfos);
			}

			thread::future<std::vector<ValidationResult>> validateAll(const model::WeakEntityInfos& entityInfos) const override {
				return validateT<AllTraits>(entityInfos);
			}

		private:
			thread::IoThreadPool& m_pool;
			std::shared_ptr<const StatelessEntityValidator> m_pValidator;
		};
	}

	std::shared_ptr<const ParallelValidationPolicy> CreateParallelValidationPolicy(
			thread::IoThreadPool& pool,
			const std::shared_ptr<const StatelessEntityValidator>& pValidator) {
		return std::make_shared<const DefaultParallelValidationPolicy>(pool, pValidator);
	}
}}
