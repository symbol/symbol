#pragma once
#include "catapult/types.h"

namespace catapult {

	/// Namespace identifier.
	struct NamespaceId_tag {};
	using NamespaceId = utils::BaseValue<uint64_t, NamespaceId_tag>;
}
