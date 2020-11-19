/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "ZeroMqUtChangeSubscriber.h"
#include "ZeroMqEntityPublisher.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqUtChangeSubscriber : public cache::UtChangeSubscriber {
		public:
			explicit ZeroMqUtChangeSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyAdds(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransaction(TransactionMarker::Unconfirmed_Transaction_Add_Marker, transactionInfo, Height());
			}

			void notifyRemoves(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransactionHash(TransactionMarker::Unconfirmed_Transaction_Remove_Marker, transactionInfo);
			}

			void flush() override {
				// empty because messages are pushed by other calls
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<cache::UtChangeSubscriber> CreateZeroMqUtChangeSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqUtChangeSubscriber>(publisher);
	}
}}
