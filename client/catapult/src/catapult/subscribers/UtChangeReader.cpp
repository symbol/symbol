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

#include "UtChangeReader.h"
#include "SubscriberOperationTypes.h"
#include "catapult/cache_tx/UtChangeSubscriber.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/io/TransactionInfoSerializer.h"

namespace catapult { namespace subscribers {

	void ReadNextUtChange(io::InputStream& inputStream, cache::UtChangeSubscriber& subscriber) {
		auto operationType = static_cast<UtChangeOperationType>(io::Read8(inputStream));
		model::TransactionInfosSet transactionInfos;
		io::ReadTransactionInfos(inputStream, transactionInfos);

		switch (operationType) {
		case UtChangeOperationType::Add:
			return subscriber.notifyAdds(transactionInfos);
		case UtChangeOperationType::Remove:
			return subscriber.notifyRemoves(transactionInfos);
		}

		CATAPULT_THROW_INVALID_ARGUMENT_1("invalid ut change operation type", static_cast<uint16_t>(operationType));
	}
}}
