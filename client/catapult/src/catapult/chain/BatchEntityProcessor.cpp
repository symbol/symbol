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
					observers::ObserverState& state) const {
				if (entityInfos.empty())
					return ValidationResult::Neutral;

				auto readOnlyCache = state.Cache.toReadOnly();
				auto resolverContext = m_config.ResolverContextFactory(readOnlyCache);
				auto validatorContext = ValidatorContext(height, timestamp, m_config.Network, resolverContext, readOnlyCache);
				auto observerContext = observers::ObserverContext(state, height, observers::NotifyMode::Commit, resolverContext);

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
