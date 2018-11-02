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
#include "catapult/handlers/HandlerTypes.h"
#include "catapult/model/CacheEntryInfo.h"

namespace catapult { namespace ionet { class ServerPacketHandlers; } }

namespace catapult { namespace handlers {

	/// Alias for a hash lock infos producer factory.
	using HashLockInfosProducerFactory = SharedPointerProducerFactory<Hash256, model::CacheEntryInfo<Hash256>>;

	/// Registers a hash lock infos handler in \a handlers that responds with hash lock infos
	/// returned by a producer from \a hashLockInfosProducerFactory.
	void RegisterHashLockInfosHandler(
			ionet::ServerPacketHandlers& handlers,
			const HashLockInfosProducerFactory& hashLockInfosProducerFactory);
}}
