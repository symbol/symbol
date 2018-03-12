#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "TransactionConsumers.h"
#include "catapult/model/TransactionUtils.h"

namespace catapult { namespace consumers {

	namespace {
		template<typename TTransactionElements>
		void UpdateAddresses(TTransactionElements& elements, const model::NotificationPublisher& notificationPublisher) {
			for (auto& element : elements) {
				auto addresses = ExtractAddresses(element.Transaction, notificationPublisher);
				element.OptionalExtractedAddresses = std::make_shared<decltype(addresses)>(std::move(addresses));
			}
		}

		class BlockAddressExtractionConsumer {
		public:
			explicit BlockAddressExtractionConsumer(const model::NotificationPublisher& notificationPublisher)
					: m_notificationPublisher(notificationPublisher)
			{}

		public:
			ConsumerResult operator()(BlockElements& elements) const {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				for (auto& element : elements)
					UpdateAddresses(element.Transactions, m_notificationPublisher);

				return Continue();
			}

		private:
			const model::NotificationPublisher& m_notificationPublisher;
		};
	}

	disruptor::BlockConsumer CreateBlockAddressExtractionConsumer(const model::NotificationPublisher& notificationPublisher) {
		return BlockAddressExtractionConsumer(notificationPublisher);
	}

	namespace {
		class TransactionAddressExtractionConsumer {
		public:
			explicit TransactionAddressExtractionConsumer(const model::NotificationPublisher& notificationPublisher)
					: m_notificationPublisher(notificationPublisher)
			{}

		public:
			ConsumerResult operator()(TransactionElements& elements) const {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				UpdateAddresses(elements, m_notificationPublisher);

				return Continue();
			}

		private:
			const model::NotificationPublisher& m_notificationPublisher;
		};
	}

	disruptor::TransactionConsumer CreateTransactionAddressExtractionConsumer(const model::NotificationPublisher& notificationPublisher) {
		return TransactionAddressExtractionConsumer(notificationPublisher);
	}
}}
