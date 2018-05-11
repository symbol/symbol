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

#include "UtSynchronizer.h"
#include "EntitiesSynchronizer.h"
#include "catapult/api/RemoteTransactionApi.h"

namespace catapult { namespace chain {

	namespace {
		struct UtTraits {
		public:
			using RemoteApiType = api::RemoteTransactionApi;
			static constexpr auto Name = "unconfirmed transactions";

		public:
			explicit UtTraits(
					const ShortHashesSupplier& shortHashesSupplier,
					const handlers::TransactionRangeHandler& transactionRangeConsumer)
					: m_shortHashesSupplier(shortHashesSupplier)
					, m_transactionRangeConsumer(transactionRangeConsumer)
			{}

		public:
			thread::future<model::TransactionRange> apiCall(const RemoteApiType& api) const {
				return api.unconfirmedTransactions(m_shortHashesSupplier());
			}

			void consume(model::TransactionRange&& range) const {
				m_transactionRangeConsumer(std::move(range));
			}

		private:
			ShortHashesSupplier m_shortHashesSupplier;
			handlers::TransactionRangeHandler m_transactionRangeConsumer;
		};
	}

	RemoteNodeSynchronizer<api::RemoteTransactionApi> CreateUtSynchronizer(
			const ShortHashesSupplier& shortHashesSupplier,
			const handlers::TransactionRangeHandler& transactionRangeConsumer) {
		auto traits = UtTraits(shortHashesSupplier, transactionRangeConsumer);
		auto pSynchronizer = std::make_shared<EntitiesSynchronizer<UtTraits>>(std::move(traits));
		return CreateRemoteNodeSynchronizer(pSynchronizer);
	}
}}
