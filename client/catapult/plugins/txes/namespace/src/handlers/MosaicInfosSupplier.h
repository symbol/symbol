#pragma once
#include "catapult/model/EntityRange.h"
#include "catapult/types.h"
#include <functional>
#include <vector>

namespace catapult {
	namespace cache { class MosaicCache; }
	namespace model { struct MosaicInfo; }
}

namespace catapult { namespace handlers {

	/// Prototype for a function that supplies a vector of mosaic infos given a range of ids.
	using MosaicInfosSupplier = std::function<std::vector<std::shared_ptr<const model::MosaicInfo>>(const model::EntityRange<MosaicId>&)>;

	/// Creates a supplier that supplies mosaic infos from \a mosaicCache.
	MosaicInfosSupplier CreateMosaicInfosSupplier(const cache::MosaicCache& mosaicCache);
}}
