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
#include "finalization/src/model/FinalizationProof.h"
#include "finalization/src/model/FinalizationStatistics.h"
#include "catapult/api/RemoteApi.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace api {

	/// Api for retrieving proof information from a remote node.
	class RemoteProofApi : public RemoteApi {
	protected:
		/// Creates a remote api for the node with specified \a remoteIdentity.
		explicit RemoteProofApi(const model::NodeIdentity& remoteIdentity) : RemoteApi(remoteIdentity)
		{}

	public:
		/// Gets the finalization statistics.
		virtual thread::future<model::FinalizationStatistics> finalizationStatistics() const = 0;

		/// Gets the finalization proof at \a point.
		virtual thread::future<std::shared_ptr<const model::FinalizationProof>> proofAt(FinalizationPoint point) const = 0;

		/// Gets the finalization proof at \a height.
		virtual thread::future<std::shared_ptr<const model::FinalizationProof>> proofAt(Height height) const = 0;
	};

	/// Creates a proof api for interacting with a remote node with the specified \a io and \a remoteIdentity.
	std::unique_ptr<RemoteProofApi> CreateRemoteProofApi(ionet::PacketIo& io, const model::NodeIdentity& remoteIdentity);
}}
