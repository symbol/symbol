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
#include "src/model/TransferNotifications.h"
#include "catapult/observers/ObserverTypes.h"

namespace catapult { namespace config { class CatapultDirectory; } }

namespace catapult { namespace observers {

	/// Observes transfer messages starting with \a marker and sent to \a recipient and writes them to \a directory.
	DECLARE_OBSERVER(TransferMessage, model::TransferMessageNotification)(
			uint64_t marker,
			const Address& recipient,
			const config::CatapultDirectory& directory);
}}
