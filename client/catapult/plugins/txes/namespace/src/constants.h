#pragma once
#include "types.h"

namespace catapult {

	/// The maximum number of parts for a namespace.
	constexpr size_t Namespace_Max_Depth = 3;

	/// Duration of eternal artifact.
	constexpr BlockDuration Eternal_Artifact_Duration(0);

	/// Base id for namespaces.
	constexpr NamespaceId Namespace_Base_Id(0);

	/// The number of rules for a mosaic levy.
	constexpr size_t Num_Mosaic_Levy_Rule_Ids = 5;

	/// The NEM namespace id.
#ifdef SIGNATURE_SCHEME_NIS1
	constexpr NamespaceId Nem_Id(0x2912FDE512A2C7B8ULL);
#else
	constexpr NamespaceId Nem_Id(0x84B3552D375FFA4BULL);
#endif
}
