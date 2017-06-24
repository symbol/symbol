#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "InputUtils.h"
#include "TransactionConsumers.h"
#include "catapult/validators/AggregateEntityValidator.h"

namespace catapult { namespace consumers {

	namespace {
		template<typename TElementsContainer>
		class StatelessValidationConsumer {
		private:
			using ExtractEntityInfosFunc = std::function<void (const TElementsContainer&, model::WeakEntityInfos&)>;

		public:
			StatelessValidationConsumer(
					const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
					const validators::ParallelValidationPolicyFunc& validationPolicy,
					const ExtractEntityInfosFunc& extractEntityInfos)
					: m_pValidator(pValidator)
					, m_validationPolicy(validationPolicy)
					, m_extractEntityInfos(extractEntityInfos)
					, m_dispatcher(pValidator->curry())
			{}

		public:
			ConsumerResult operator()(const TElementsContainer& elements) const {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				model::WeakEntityInfos entityInfos;
				m_extractEntityInfos(elements, entityInfos);

				auto result = m_dispatcher.dispatch(m_validationPolicy, entityInfos).get();

				if (IsValidationResultSuccess(result))
					return Continue();

				CATAPULT_LOG_LEVEL(validators::MapToLogLevel(result)) << "transaction validation failed: " << result;
				return Abort(result);
			}

		private:
			std::shared_ptr<const validators::stateless::AggregateEntityValidator> m_pValidator;
			validators::ParallelValidationPolicyFunc m_validationPolicy;
			ExtractEntityInfosFunc m_extractEntityInfos;
			validators::stateless::AggregateEntityValidator::DispatchForwarder m_dispatcher;
		};
	}

	disruptor::ConstBlockConsumer CreateBlockStatelessValidationConsumer(
			const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
			const validators::ParallelValidationPolicyFunc& validationPolicy,
			const RequiresValidationPredicate& requiresValidationPredicate) {
		return StatelessValidationConsumer<BlockElements>(
				pValidator,
				validationPolicy,
				[requiresValidationPredicate](const auto& elements, auto& entityInfos) {
					ExtractMatchingEntityInfos(elements, entityInfos, requiresValidationPredicate);
				});
	}

	disruptor::ConstTransactionConsumer CreateTransactionStatelessValidationConsumer(
			const std::shared_ptr<const validators::stateless::AggregateEntityValidator>& pValidator,
			const validators::ParallelValidationPolicyFunc& validationPolicy) {
		return StatelessValidationConsumer<TransactionElements>(
				pValidator,
				validationPolicy,
				[](const auto& elements, auto& entityInfos) { ExtractEntityInfos(elements, entityInfos); });
	}
}}
