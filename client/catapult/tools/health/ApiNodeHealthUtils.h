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
#include "catapult/api/ChainApi.h"
#include "catapult/thread/Future.h"

namespace catapult {
	namespace ionet { class Node; }
	namespace thread { class IoThreadPool; }
}

namespace catapult { namespace tools { namespace health {

	/// Creates a future for retrieving the chain statistics of the specified api \a node over REST API using \a pool.
	/// \note Default REST API port is assumed.
	thread::future<api::ChainStatistics> CreateApiNodeChainStatisticsFuture(thread::IoThreadPool& pool, const ionet::Node& node);
}}}
