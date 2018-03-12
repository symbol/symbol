#pragma once
#include "catapult/model/Elements.h"

namespace catapult { namespace parsers {

	/// Parses a block element out of \a buffer and updates \a numBytesConsumed with the number of buffer bytes consumed.
	model::BlockElement ParseBlockElement(const RawBuffer& buffer, size_t& numBytesConsumed);
}}
