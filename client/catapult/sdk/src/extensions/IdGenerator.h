#pragma once
#include "plugins/txes/namespace/src/model/NamespaceConstants.h"
#include "catapult/utils/Array.h"
#include "catapult/types.h"

namespace catapult { namespace extensions {

	/// Generates a mosaic id given a unified mosaic \a name.
	MosaicId GenerateMosaicId(const RawString& name);

	/// A namespace path.
	using NamespacePath = utils::Array<NamespaceId, Namespace_Max_Depth>;

	/// Parses a unified namespace \a name into a path.
	NamespacePath GenerateNamespacePath(const RawString& name);
}}
