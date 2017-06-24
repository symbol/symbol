#include "SizePrefixedEntity.h"
#include <cstring>

namespace catapult { namespace model {

	const uint8_t* SizePrefixedEntity::ToBytePointer() const {
		return reinterpret_cast<const uint8_t*>(this);
	}

	uint8_t* SizePrefixedEntity::ToBytePointer() {
		return reinterpret_cast<uint8_t*>(this);
	}

	bool SizePrefixedEntity::operator==(const SizePrefixedEntity& rhs) const {
		return Size == rhs.Size && 0 == std::memcmp(this, &rhs, Size);
	}

	bool SizePrefixedEntity::operator!=(const SizePrefixedEntity& rhs) const {
		return !(*this == rhs);
	}
}}
