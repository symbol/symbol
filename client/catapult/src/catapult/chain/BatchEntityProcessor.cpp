#include "BatchEntityProcessor.h"
#include "ProcessingNotificationSubscriber.h"
#include "catapult/cache/CatapultCache.h"

using namespace catapult::validators;

namespace catapult { namespace chain {

	namespace {
		class DefaultBatchEntityProcessor {
		public:
			explicit DefaultBatchEntityProcessor(const ExecutionConfiguration& config) : m_config(config)
			{}

		public:
			ValidationResult operator()(
					Height height,
					Timestamp timestamp,
					const model::WeakEntityInfos& entityInfos,
					const observers::ObserverState& state) const {
				if (entityInfos.empty())
					return ValidationResult::Neutral;

				auto readOnlyCache = state.Cache.toReadOnly();
				auto validatorContext = ValidatorContext(height, timestamp, m_config.Network, readOnlyCache);
				auto observerContext = observers::ObserverContext(state, height, observers::NotifyMode::Commit);

				ProcessingNotificationSubscriber sub(*m_config.pValidator, validatorContext, *m_config.pObserver, observerContext);
				for (const auto& entityInfo : entityInfos) {
					m_config.pNotificationPublisher->publish(entityInfo, sub);
					if (!IsValidationResultSuccess(sub.result()))
						return sub.result();
				}

				return ValidationResult::Success;
			}

		private:
			ExecutionConfiguration m_config;
		};
	}

	BatchEntityProcessor CreateBatchEntityProcessor(const ExecutionConfiguration& config) {
		return DefaultBatchEntityProcessor(config);
	}
}}
