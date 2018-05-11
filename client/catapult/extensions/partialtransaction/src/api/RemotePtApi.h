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
#include "partialtransaction/src/PtTypes.h"
#include "catapult/cache/ShortHashPair.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace api {

	/// An api for retrieving partial transaction information from a remote node.
	class RemotePtApi {
	public:
		virtual ~RemotePtApi() {}

	public:
		/// Gets all partial transaction infos from the remote excluding those with all hashes in \a knownShortHashPairs.
		virtual thread::future<partialtransaction::CosignedTransactionInfos> transactionInfos(
				cache::ShortHashPairRange&& knownShortHashPairs) const = 0;
	};

	/// Creates a partial transaction api for interacting with a remote node with the specified \a io
	/// and transaction \a registry composed of supported transactions.
	std::unique_ptr<RemotePtApi> CreateRemotePtApi(ionet::PacketIo& io, const model::TransactionRegistry& registry);
}}
