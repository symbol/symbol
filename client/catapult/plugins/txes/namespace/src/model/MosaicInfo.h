#pragma once
#include "ArtifactInfoAttributes.h"
#include "NamespaceConstants.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a mosaic info.
	struct MosaicInfo {
		/// The namespace id.
		catapult::NamespaceId NamespaceId;

		/// The mosaic id.
		MosaicId Id;

		/// The mosaic attributes.
		ArtifactInfoAttributes Attributes;

		/// The supply.
		Amount Supply;
	};

#pragma pack(pop)
}}
