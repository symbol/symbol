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
#include "MemoryUtCache.h"

namespace catapult { namespace cache {

	/// Retrieves the number of transactions contained within a top-level transaction.
	using EmbeddedCountRetriever = std::function<uint32_t (const model::Transaction&)>;

	/// Gets the pointers to the first \a transactionLimit transaction infos in \a utCacheView
	/// where \a countRetriever returns the total number of transactions contained within a top-level transaction.
	/// \note Pointers are only safe to access during the lifetime of \a utCacheView.
	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t transactionLimit,
			const EmbeddedCountRetriever& countRetriever);

	/// Gets the pointers to the first \a transactionLimit transaction infos in \a utCacheView that pass \a filter
	/// where \a countRetriever returns the total number of transactions contained within a top-level transaction.
	/// \note Pointers are only safe to access during the lifetime of \a utCacheView.
	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t transactionLimit,
			const EmbeddedCountRetriever& countRetriever,
			const predicate<const model::TransactionInfo&>& filter);

	/// Gets the pointers to the first \a transactionLimit transaction infos in \a utCacheView that pass \a filter after sorting
	/// by \a sortComparer where \a countRetriever returns the total number of transactions contained within a top-level transaction.
	/// \note Pointers are only safe to access during the lifetime of \a utCacheView.
	std::vector<const model::TransactionInfo*> GetFirstTransactionInfoPointers(
			const MemoryUtCacheView& utCacheView,
			uint32_t transactionLimit,
			const EmbeddedCountRetriever& countRetriever,
			const predicate<const model::TransactionInfo*, const model::TransactionInfo*>& sortComparer,
			const predicate<const model::TransactionInfo&>& filter);
}}
