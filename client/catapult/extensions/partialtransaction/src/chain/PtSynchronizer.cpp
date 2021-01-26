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

#include "PtSynchronizer.h"
#include "partialtransaction/src/api/RemotePtApi.h"
#include "catapult/chain/EntitiesSynchronizer.h"

namespace catapult { namespace chain {

	namespace {
		struct PtTraits {
		public:
			using RemoteApiType = api::RemotePtApi;
			static constexpr auto Name = "partial transactions";

		public:
			PtTraits(
					const TimeSupplier& timeSupplier,
					const partialtransaction::ShortHashPairsSupplier& shortHashPairsSupplier,
					const partialtransaction::CosignedTransactionInfosConsumer& transactionInfosConsumer)
					: m_timeSupplier(timeSupplier)
					, m_shortHashPairsSupplier(shortHashPairsSupplier)
					, m_transactionInfosConsumer(transactionInfosConsumer)
			{}

		public:
			thread::future<partialtransaction::CosignedTransactionInfos> apiCall(const RemoteApiType& api) const {
				return api.transactionInfos(m_timeSupplier(), m_shortHashPairsSupplier());
			}

			void consume(partialtransaction::CosignedTransactionInfos&& transactionInfos, const model::NodeIdentity&) const {
				m_transactionInfosConsumer(std::move(transactionInfos));
			}

		private:
			TimeSupplier m_timeSupplier;
			partialtransaction::ShortHashPairsSupplier m_shortHashPairsSupplier;
			partialtransaction::CosignedTransactionInfosConsumer m_transactionInfosConsumer;
		};
	}

	RemoteNodeSynchronizer<api::RemotePtApi> CreatePtSynchronizer(
			const TimeSupplier& timeSupplier,
			const partialtransaction::ShortHashPairsSupplier& shortHashPairsSupplier,
			const partialtransaction::CosignedTransactionInfosConsumer& transactionInfosConsumer,
			const predicate<>& shouldExecute) {
		auto traits = PtTraits(timeSupplier, shortHashPairsSupplier, transactionInfosConsumer);
		auto pSynchronizer = std::make_shared<EntitiesSynchronizer<PtTraits>>(std::move(traits));
		return CreateConditionalRemoteNodeSynchronizer(pSynchronizer, shouldExecute);
	}
}}
