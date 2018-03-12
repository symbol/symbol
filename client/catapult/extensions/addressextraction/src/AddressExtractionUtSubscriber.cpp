#include "AddressExtractionUtSubscriber.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/TransactionUtils.h"

namespace catapult { namespace addressextraction {

	namespace {
		class AddressExtractionChangeSubscriber : public cache::UtChangeSubscriber {
		public:
			explicit AddressExtractionChangeSubscriber(std::unique_ptr<model::NotificationPublisher>&& pPublisher)
					: m_pPublisher(std::move(pPublisher))
			{}

		public:
			void notifyAdds(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos) {
					if (transactionInfo.OptionalExtractedAddresses)
						continue;

					auto addresses = model::ExtractAddresses(*transactionInfo.pEntity, *m_pPublisher);

					auto& mutableTransactionInfo = const_cast<model::TransactionInfo&>(transactionInfo);
					mutableTransactionInfo.OptionalExtractedAddresses = std::make_shared<model::AddressSet>(std::move(addresses));
				}
			}

			void notifyRemoves(const TransactionInfos&) override {
				// removes originate from the cache, so don't need to be modified
			}

			void flush() override {
				// nothing to intercept
			}

		private:
			std::unique_ptr<model::NotificationPublisher> m_pPublisher;
		};
	}

	std::unique_ptr<cache::UtChangeSubscriber> CreateAddressExtractionChangeSubscriber(
			std::unique_ptr<model::NotificationPublisher>&& pPublisher) {
		return std::make_unique<AddressExtractionChangeSubscriber>(std::move(pPublisher));
	}
}}
