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

#include "BlockConsumers.h"
#include "ConsumerResults.h"
#include "TransactionConsumers.h"
#include "ValidationConsumerUtils.h"
#include "catapult/model/NotificationSubscriber.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/thread/ParallelFor.h"
#include "catapult/validators/AggregateValidationResult.h"

namespace catapult { namespace consumers {

	namespace {
		class SignatureCapturingNotificationSubscriber : public model::NotificationSubscriber {
		public:
			explicit SignatureCapturingNotificationSubscriber(const GenerationHashSeed& generationHashSeed)
					: m_generationHashSeed(generationHashSeed)
					, m_entityIndex(0)
			{}

		public:
			const auto& notificationToEntityIndexMap() const {
				return m_notificationToEntityIndexMap;
			}

			const auto& inputs() const {
				return m_inputs;
			}

		public:
			void next() {
				++m_entityIndex;
			}

		public:
			void notify(const model::Notification& notification) override {
				if (model::SignatureNotification::Notification_Type != notification.Type)
					return;

				m_notificationToEntityIndexMap.push_back(m_entityIndex);
				add(static_cast<const model::SignatureNotification&>(notification));
			}

		private:
			void add(const model::SignatureNotification& notification) {
				std::vector<RawBuffer> buffers;
				if (model::SignatureNotification::ReplayProtectionMode::Enabled == notification.DataReplayProtectionMode)
					buffers.push_back(m_generationHashSeed);

				buffers.push_back(notification.Data);

				m_inputs.push_back({ notification.SignerPublicKey, buffers, notification.Signature });
			}

		private:
			const GenerationHashSeed& m_generationHashSeed;
			size_t m_entityIndex;
			std::vector<size_t> m_notificationToEntityIndexMap;
			std::vector<crypto::SignatureInput> m_inputs;
		};

		std::unique_ptr<SignatureCapturingNotificationSubscriber> ExtractAllSignatureNotifications(
				const GenerationHashSeed& generationHashSeed,
				const model::NotificationPublisher& publisher,
				const model::WeakEntityInfos& entityInfos) {
			auto pSub = std::make_unique<SignatureCapturingNotificationSubscriber>(generationHashSeed);
			for (const auto& entityInfo : entityInfos) {
				publisher.publish(entityInfo, *pSub);
				pSub->next();
			}

			return pSub;
		}

		std::vector<validators::ValidationResult> MapNotificationResultsToEntityResults(
				size_t numEntities,
				const std::vector<size_t>& notificationToEntityIndexMap,
				const std::vector<validators::ValidationResult>& notificationResults) {
			std::vector<validators::ValidationResult> entityResults(numEntities, validators::ValidationResult::Success);
			for (auto i = 0u; i < notificationResults.size(); ++i) {
				if (IsValidationResultFailure(notificationResults[i]))
					entityResults[notificationToEntityIndexMap[i]] = notificationResults[i];
			}

			return entityResults;
		}
	}

	disruptor::ConstBlockConsumer CreateBlockBatchSignatureConsumer(
			const GenerationHashSeed& generationHashSeed,
			const crypto::RandomFiller& randomFiller,
			const std::shared_ptr<const model::NotificationPublisher>& pPublisher,
			const std::shared_ptr<thread::IoThreadPool>& pPool,
			const RequiresValidationPredicate& requiresValidationPredicate) {
		return MakeBlockValidationConsumer(requiresValidationPredicate, [generationHashSeed, randomFiller, pPublisher, pPool](
				const auto& entityInfos) {
			// find all signature notifications
			auto inputs = ExtractAllSignatureNotifications(generationHashSeed, *pPublisher, entityInfos)->inputs();

			// process signatures in batches
			std::atomic<validators::ValidationResult> aggregateResult(validators::ValidationResult::Success);
			auto partitionCallback = [&randomFiller, &aggregateResult](auto itBegin, auto itEnd, auto, auto) {
				auto count = static_cast<size_t>(std::distance(itBegin, itEnd));
				if (!VerifyMultiShortCircuit(randomFiller, &*itBegin, count))
					validators::AggregateValidationResult(aggregateResult, Failure_Consumer_Batch_Signature_Not_Verifiable);
			};

			thread::ParallelForPartition(pPool->ioContext(), inputs, pPool->numWorkerThreads(), partitionCallback).get();
			return aggregateResult.load();
		});
	}

	disruptor::TransactionConsumer CreateTransactionBatchSignatureConsumer(
			const GenerationHashSeed& generationHashSeed,
			const crypto::RandomFiller& randomFiller,
			const std::shared_ptr<const model::NotificationPublisher>& pPublisher,
			const std::shared_ptr<thread::IoThreadPool>& pPool,
			const chain::FailedTransactionSink& failedTransactionSink) {
		return MakeTransactionValidationConsumer(failedTransactionSink, [generationHashSeed, randomFiller, pPublisher, pPool](
				const auto& entityInfos) {
			// find all signature notifications
			auto pSub = ExtractAllSignatureNotifications(generationHashSeed, *pPublisher, entityInfos);

			// process signatures in batches
			// note: store notification (not entity) results because it's possible for an entity to be split across partitions,
			//       which would lead to a write data race (of same data) from multiple threads
			std::vector<validators::ValidationResult> notificationResults(pSub->inputs().size(), validators::ValidationResult::Success);
			auto partitionCallback = [&randomFiller, &notificationResults](auto itBegin, auto itEnd, auto startIndex, auto) {
				auto count = static_cast<size_t>(std::distance(itBegin, itEnd));
				auto partitionResultsPair = VerifyMulti(randomFiller, &*itBegin, count);
				if (partitionResultsPair.second)
					return;

				auto index = startIndex;
				for (auto result : partitionResultsPair.first) {
					if (!result)
						notificationResults[index] = Failure_Consumer_Batch_Signature_Not_Verifiable;

					++index;
				}
			};

			thread::ParallelForPartition(pPool->ioContext(), pSub->inputs(), pPool->numWorkerThreads(), partitionCallback).get();

			return MapNotificationResultsToEntityResults(entityInfos.size(), pSub->notificationToEntityIndexMap(), notificationResults);
		});
	}
}}
