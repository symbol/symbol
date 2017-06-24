#pragma once
#include "MosaicEntry.h"
#include "src/constants.h"

namespace catapult { namespace state { class MosaicEntry; } }

namespace catapult { namespace state {

	/// A mosaic descriptor.
	struct MosaicDescriptor {
	public:
		/// Creates a mosaic descriptor around \a entry, \a index and \a isActive.
		explicit MosaicDescriptor(
				const std::shared_ptr<const MosaicEntry>& pMosaicEntry,
				uint32_t index,
				bool isActive)
				: pEntry(pMosaicEntry)
				, Index(index)
				, IsActive(isActive)
		{}

	public:
		/// The mosaic entry.
		const std::shared_ptr<const MosaicEntry> pEntry;

		/// The index in the mosaic history.
		uint32_t Index;

		/// Flag indicating whether or not the mosaic is active.
		bool IsActive;
	};
}}
