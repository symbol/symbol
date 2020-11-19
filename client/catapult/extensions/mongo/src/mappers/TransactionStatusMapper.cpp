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

#include "TransactionStatusMapper.h"
#include "MapperUtils.h"
#include "catapult/model/TransactionStatus.h"

namespace catapult { namespace mongo { namespace mappers {

	bsoncxx::document::value ToDbModel(const model::TransactionStatus& status) {
		bson_stream::document builder;
		return builder
				<< "status" << bson_stream::open_document
					<< "hash" << ToBinary(status.Hash)
					<< "code" << static_cast<int32_t>(status.Status)
					<< "deadline" << ToInt64(status.Deadline)
				<< bson_stream::close_document
				<< bson_stream::finalize;
	}
}}}
