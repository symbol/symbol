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
#include "ContainerTypes.h"
#include "EntityInfo.h"

namespace catapult { namespace model {

	/// Tracks transaction changes and keeps track of \em net changes.
	class TransactionChangeTracker {
	public:
		/// Gets the infos of net added transactions.
		const TransactionInfosSet& addedTransactionInfos() const {
			return m_addedTransactionInfos;
		}

		/// Gets the infos of net removed transactions.
		const TransactionInfosSet& removedTransactionInfos() const {
			return m_removedTransactionInfos;
		}

	public:
		/// Marks transaction info (\a transactionInfo) as added.
		void add(const TransactionInfo& transactionInfo) {
			// if the info can be found in the set of deleted transactions, it means that
			// it is currently in the collection so removing the info from the set means (re)adding it
			auto iter = m_removedTransactionInfos.find(transactionInfo);
			if (m_removedTransactionInfos.cend() != iter)
				m_removedTransactionInfos.erase(iter);
			else
				m_addedTransactionInfos.emplace(transactionInfo.copy());
		}

		/// Marks transaction info (\a transactionInfo) as removed.
		void remove(const TransactionInfo& transactionInfo) {
			// if the info can be found in the set of added transactions, simply remove it
			// this only works because transactions are immutable (there are no upserts)
			// so a newly saved transaction cannot already be in the database
			auto iter = m_addedTransactionInfos.find(transactionInfo);
			if (m_addedTransactionInfos.cend() != iter)
				m_addedTransactionInfos.erase(iter);
			else
				m_removedTransactionInfos.emplace(transactionInfo.copy());
		}

		/// Clears all added and removed transactions.
		void reset() {
			m_addedTransactionInfos.clear();
			m_removedTransactionInfos.clear();
		}

	private:
		TransactionInfosSet m_addedTransactionInfos;
		TransactionInfosSet m_removedTransactionInfos;
	};
}}
