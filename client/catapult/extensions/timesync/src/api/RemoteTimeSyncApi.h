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
#include "timesync/src/CommunicationTimestamps.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace ionet { class PacketIo; } }

namespace catapult { namespace api {

	/// Api for retrieving communication timestamps from a remote node.
	class RemoteTimeSyncApi {
	public:
		virtual ~RemoteTimeSyncApi() = default;

	public:
		/// Gets the communication timestamps from a remote node.
		virtual thread::future<timesync::CommunicationTimestamps> networkTime() const = 0;
	};

	/// Creates a time sync api for interacting with a remote node with the specified \a io.
	std::unique_ptr<RemoteTimeSyncApi> CreateRemoteTimeSyncApi(ionet::PacketIo& io);
}}
