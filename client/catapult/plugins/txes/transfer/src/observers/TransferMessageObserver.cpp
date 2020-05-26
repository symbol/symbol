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

#include "Observers.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/FileQueue.h"
#include "catapult/io/PodIoUtils.h"

namespace catapult { namespace observers {

	namespace {
		constexpr auto Marker_Size = sizeof(uint64_t);

		using Notification = model::TransferMessageNotification;
	}

	DECLARE_OBSERVER(TransferMessage, Notification)(
			uint64_t marker,
			const Address& recipient,
			const config::CatapultDirectory& directory) {
		return MAKE_OBSERVER(TransferMessage, Notification, ([marker, recipient, directory](
				const Notification& notification,
				ObserverContext& context) {
			if (notification.MessageSize <= Marker_Size || marker != reinterpret_cast<const uint64_t&>(*notification.MessagePtr))
				return;

			if (recipient != context.Resolvers.resolve(notification.Recipient))
				return;

			io::FileQueueWriter writer(directory.str());
			io::Write8(writer, NotifyMode::Commit == context.Mode ? 0 : 1);
			writer.write(notification.SenderPublicKey);
			writer.write({ notification.MessagePtr + Marker_Size, notification.MessageSize - Marker_Size });
			writer.flush();
		}));
	}
}}
