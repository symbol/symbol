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
