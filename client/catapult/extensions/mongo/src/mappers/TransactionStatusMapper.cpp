#include "TransactionStatusMapper.h"
#include "MapperUtils.h"
#include "catapult/model/TransactionStatus.h"

namespace catapult { namespace mongo { namespace mappers {

	bsoncxx::document::value ToDbModel(const model::TransactionStatus& status) {
		bson_stream::document builder;
		return builder
				<< "hash" << ToBinary(status.Hash)
				<< "status" << static_cast<int32_t>(status.Status)
				<< "deadline" << ToInt64(status.Deadline)
				<< bson_stream::finalize;
	}
}}}
