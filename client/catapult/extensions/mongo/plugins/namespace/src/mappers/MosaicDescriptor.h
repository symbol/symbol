#pragma once
#include "plugins/txes/namespace/src/state/MosaicEntry.h"

namespace catapult { namespace mongo { namespace plugins {

	/// A mosaic descriptor.
	struct MosaicDescriptor {
	public:
		/// Creates a mosaic descriptor around \a entry, \a index and \a isActive.
		explicit MosaicDescriptor(const std::shared_ptr<const state::MosaicEntry>& pMosaicEntry, uint32_t index, bool isActive)
				: pEntry(pMosaicEntry)
				, Index(index)
				, IsActive(isActive)
		{}

	public:
		/// The mosaic entry.
		const std::shared_ptr<const state::MosaicEntry> pEntry;

		/// The index in the mosaic history.
		uint32_t Index;

		/// Flag indicating whether or not the mosaic is active.
		bool IsActive;
	};
}}}
