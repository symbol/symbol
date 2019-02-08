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
#include "RemoteApi.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace api {

	/// An api for retrieving transaction information from a remote node.
	class RemoteTransactionApi : public RemoteApi {
	protected:
		/// Creates a remote api for the node with specified public key (\a remotePublicKey).
		explicit RemoteTransactionApi(const Key& remotePublicKey) : RemoteApi(remotePublicKey)
		{}

	public:
		/// Gets all unconfirmed transactions from the remote that have a fee multiplier at least \a minFeeMultiplier
		/// and do not have a short hash in \a knownShortHashes.
		virtual thread::future<model::TransactionRange> unconfirmedTransactions(
				BlockFeeMultiplier minFeeMultiplier,
				model::ShortHashRange&& knownShortHashes) const = 0;
	};

	/// Creates a transaction api for interacting with a remote node with the specified \a io with public key (\a remotePublicKey)
	/// and transaction \a registry composed of supported transactions.
	std::unique_ptr<RemoteTransactionApi> CreateRemoteTransactionApi(
			ionet::PacketIo& io,
			const Key& remotePublicKey,
			const model::TransactionRegistry& registry);
}}
