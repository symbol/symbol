#pragma once
#include "catapult/types.h"

namespace catapult {

	/// Namespace identifier.
	struct NamespaceId_tag {};
	using NamespaceId = utils::BaseValue<uint64_t, NamespaceId_tag>;

	/// Artifact duration type.
	struct ArtifactDuration_tag {};
	using ArtifactDuration = utils::BaseValue<NamespaceId::ValueType, ArtifactDuration_tag>;
}
