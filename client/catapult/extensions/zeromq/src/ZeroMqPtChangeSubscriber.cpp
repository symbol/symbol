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

#include "ZeroMqPtChangeSubscriber.h"
#include "ZeroMqEntityPublisher.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqPtChangeSubscriber : public cache::PtChangeSubscriber {
		public:
			explicit ZeroMqPtChangeSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyAddPartials(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransaction(TransactionMarker::Partial_Transaction_Add_Marker, transactionInfo, Height());
			}

			void notifyAddCosignature(
					const model::TransactionInfo& parentTransactionInfo,
					const model::Cosignature& cosignature) override {
				m_publisher.publishCosignature(parentTransactionInfo, cosignature);
			}

			void notifyRemovePartials(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransactionHash(TransactionMarker::Partial_Transaction_Remove_Marker, transactionInfo);
			}

			void flush() override {
				// empty because messages are pushed by other calls
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<cache::PtChangeSubscriber> CreateZeroMqPtChangeSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqPtChangeSubscriber>(publisher);
	}
}}
