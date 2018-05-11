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

namespace catapult {
	namespace io {
		class BlockStorage;
		class LightBlockStorage;
	}
	namespace model { class NotificationPublisher; }
}

namespace catapult { namespace mongo {

	/// Prepares (mongo) block storage (\a destinationStorage) by copying the nemesis block from \a sourceStorage and extracting
	/// notifications using \a notificationPublisher.
	void PrepareMongoBlockStorage(
			io::LightBlockStorage& destinationStorage,
			const io::BlockStorage& sourceStorage,
			const model::NotificationPublisher& notificationPublisher);
}}
