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
