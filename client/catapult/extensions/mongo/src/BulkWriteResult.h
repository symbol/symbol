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
#include <mongocxx/result/bulk_write.hpp>

namespace catapult { namespace mongo {

	/// Result of a bulk write operation to the database.
	struct BulkWriteResult {
	public:
		/// Creates a default bulk write result.
		BulkWriteResult()
				: NumInserted(0)
				, NumMatched(0)
				, NumModified(0)
				, NumDeleted(0)
				, NumUpserted(0)
		{}

		/// Creates a bulk result from a mongo \a result.
		explicit BulkWriteResult(const mongocxx::result::bulk_write& result)
				: NumInserted(result.inserted_count())
				, NumMatched(result.matched_count())
				, NumModified(result.modified_count())
				, NumDeleted(result.deleted_count())
				, NumUpserted(result.upserted_count())
		{}

	public:
		/// Aggregates all bulk write results in \a results into a single result.
		static BulkWriteResult Aggregate(const std::vector<BulkWriteResult>& results) {
			BulkWriteResult aggregate;
			for (const auto& result : results) {
				aggregate.NumInserted += result.NumInserted;
				aggregate.NumMatched += result.NumMatched;
				aggregate.NumModified += result.NumModified;
				aggregate.NumDeleted += result.NumDeleted;
				aggregate.NumUpserted += result.NumUpserted;
			}

			return aggregate;
		}

	public:
		/// Number of documents that were inserted.
		int32_t NumInserted;

		/// Number of documents that matched existing documents.
		int32_t NumMatched;

		/// Number of existing documents that were modified.
		int32_t NumModified;

		/// Number of existing documents that were deleted.
		int32_t NumDeleted;

		/// Number of documents that were inserted because no document matched.
		int32_t NumUpserted;
	};
}}
