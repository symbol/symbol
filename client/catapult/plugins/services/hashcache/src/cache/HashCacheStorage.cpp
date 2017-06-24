#include "HashCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace cache {

	void HashCacheStorage::Save(const ValueType& value, io::OutputStream& output) {
		io::Write(output, value.Time);
		io::Write(output, value.Hash);
	}

	void HashCacheStorage::Load(io::InputStream& input, DestinationType& cacheDelta) {
		state::TimestampedHash timestampedHash;
		io::Read(input, timestampedHash.Time);
		io::Read(input, timestampedHash.Hash);
		cacheDelta.insert(timestampedHash);
	}
}}
