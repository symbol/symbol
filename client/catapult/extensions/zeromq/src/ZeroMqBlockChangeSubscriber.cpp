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

#include "ZeroMqBlockChangeSubscriber.h"
#include "ZeroMqEntityPublisher.h"
#include "catapult/model/Elements.h"
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqBlockChangeSubscriber : public io::BlockChangeSubscriber {
		public:
			explicit ZeroMqBlockChangeSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				// block header
				m_publisher.publishBlockHeader(blockElement);

				// transactions
				auto height = blockElement.Block.Height;
				for (const auto& transactionElement : blockElement.Transactions)
					m_publisher.publishTransaction(TransactionMarker::Transaction_Marker, transactionElement, height);
			}

			void notifyDropBlocksAfter(Height height) override {
				m_publisher.publishDropBlocks(height);
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<io::BlockChangeSubscriber> CreateZeroMqBlockChangeSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqBlockChangeSubscriber>(publisher);
	}
}}
