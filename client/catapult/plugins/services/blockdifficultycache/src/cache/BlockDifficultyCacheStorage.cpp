#include "BlockDifficultyCacheStorage.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace cache {

	void BlockDifficultyCacheStorage::Save(const ValueType& value, io::OutputStream& output) {
		io::Write(output, value.BlockHeight);
		io::Write(output, value.BlockTimestamp);
		io::Write(output, value.BlockDifficulty);
	}

	void BlockDifficultyCacheStorage::Load(io::InputStream& input, DestinationType& cacheDelta) {
		state::BlockDifficultyInfo info;
		io::Read(input, info.BlockHeight);
		io::Read(input, info.BlockTimestamp);
		io::Read(input, info.BlockDifficulty);
		cacheDelta.insert(info);
	}
}}
