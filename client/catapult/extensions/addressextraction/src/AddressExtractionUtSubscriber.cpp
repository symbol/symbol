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
