#pragma once
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a mosaic.
	struct Mosaic {
		/// The mosaic id.
		catapult::MosaicId MosaicId;

		/// The mosaic amount.
		catapult::Amount Amount;
	};

#pragma pack(pop)
}}
