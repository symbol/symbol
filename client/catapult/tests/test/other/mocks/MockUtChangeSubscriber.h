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

#pragma once
#include "catapult/cache_tx/UtChangeSubscriber.h"
#include "tests/test/cache/AggregateTransactionsCacheTestUtils.h"

namespace catapult { namespace mocks {

	/// Mock unconfirmed transactions change subscriber flush params.
	struct UtFlushInfo {
	public:
		/// Number of added transaction infos.
		size_t NumAdds;

		/// Number of removed transaction infos.
		size_t NumRemoves;

	public:
		/// Returns \c true if this flush info is equal to \a rhs.
		constexpr bool operator==(const UtFlushInfo& rhs) const {
			return NumAdds == rhs.NumAdds && NumRemoves == rhs.NumRemoves;
		}
	};

	/// Mock unconfirmed transactions change subscriber.
	class MockUtChangeSubscriber : public test::MockTransactionsChangeSubscriber<cache::UtChangeSubscriber, UtFlushInfo> {
	public:
		void notifyAdds(const TransactionInfos& transactionInfos) override {
			for (const auto& transactionInfo : transactionInfos)
				m_addedInfos.push_back(transactionInfo.copy());
		}

		void notifyRemoves(const TransactionInfos& transactionInfos) override {
			for (const auto& transactionInfo : transactionInfos)
				m_removedInfos.push_back(transactionInfo.copy());
		}

	private:
		UtFlushInfo createFlushInfo() const override {
			return { m_addedInfos.size(), m_removedInfos.size() };
		}
	};
}}
