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

#include "TransactionStatementMapper.h"
#include "ReceiptMapper.h"
#include "catapult/model/TransactionStatement.h"

namespace catapult { namespace mongo { namespace mappers {

	namespace {
		void StreamReceipts(
				bson_stream::document& builder,
				const model::TransactionStatement& statement,
				const MongoReceiptRegistry& receiptRegistry) {
			auto receiptsArray = builder << "receipts" << bson_stream::open_array;
			for (auto i = 0u; i < statement.size(); ++i) {
				const auto& receipt = statement.receiptAt(i);
				bsoncxx::builder::stream::document receiptBuilder;
				StreamReceipt(receiptBuilder, receipt, receiptRegistry);
				receiptsArray << receiptBuilder;
			}

			receiptsArray << bson_stream::close_array;
		}
	}

	bsoncxx::document::value ToDbModel(
			Height height,
			const model::TransactionStatement& statement,
			const MongoReceiptRegistry& receiptRegistry) {
		bson_stream::document builder;
		auto doc = builder
				<< "statement" << bson_stream::open_document
					<< "height" << ToInt64(height)
					<< "source" << bson_stream::open_document
						<< "primaryId" << static_cast<int32_t>(statement.source().PrimaryId)
						<< "secondaryId" << static_cast<int32_t>(statement.source().SecondaryId)
					<< bson_stream::close_document;

		StreamReceipts(builder, statement, receiptRegistry);
		doc << bson_stream::close_document;
		return builder << bson_stream::finalize;
	}
}}}
