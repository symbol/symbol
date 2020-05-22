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

#pragma once
#include "catapult/cache_tx/PtChangeSubscriber.h"
#include "catapult/model/Cosignature.h"
#include "tests/test/cache/AggregateTransactionsCacheTestUtils.h"

namespace catapult { namespace mocks {

	/// Mock partial transactions change subscriber flush params.
	struct PtFlushInfo {
	public:
		/// Number of added partial transaction infos.
		size_t NumAdds;

		/// Number of added cosignatures.
		size_t NumCosignatureAdds;

		/// Number of removed partial transaction infos.
		size_t NumRemoves;

	public:
		/// Returns \c true if this flush info is equal to \a rhs.
		constexpr bool operator==(const PtFlushInfo& rhs) const {
			return NumAdds == rhs.NumAdds && NumCosignatureAdds == rhs.NumCosignatureAdds && NumRemoves == rhs.NumRemoves;
		}
	};

	/// Mock partial transactions change subscriber.
	class MockPtChangeSubscriber : public test::MockTransactionsChangeSubscriber<cache::PtChangeSubscriber, PtFlushInfo> {
	private:
		using CosignatureInfo = std::pair<std::unique_ptr<const model::TransactionInfo>, model::Cosignature>;

	public:
		/// Gets the added cosignatures.
		const std::vector<CosignatureInfo>& addedCosignatureInfos() const {
			return m_addedCosignatureInfos;
		}

	public:
		void notifyAddPartials(const TransactionInfos& transactionInfos) override {
			for (const auto& transactionInfo : transactionInfos)
				m_addedInfos.push_back(transactionInfo.copy());
		}

		void notifyRemovePartials(const TransactionInfos& transactionInfos) override {
			for (const auto& transactionInfo : transactionInfos)
				m_removedInfos.push_back(transactionInfo.copy());
		}

	public:
		void notifyAddCosignature(const model::TransactionInfo& parentTransactionInfo, const model::Cosignature& cosignature) override {
			m_addedCosignatureInfos.emplace_back(std::make_unique<model::TransactionInfo>(parentTransactionInfo.copy()), cosignature);
		}

	private:
		PtFlushInfo createFlushInfo() const override {
			return { m_addedInfos.size(), m_addedCosignatureInfos.size(), m_removedInfos.size() };
		}

	private:
		std::vector<CosignatureInfo> m_addedCosignatureInfos;
	};
}}
