/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
		/// Mosaic entry.
		const std::shared_ptr<const state::MosaicEntry> pEntry;

		/// Index in the mosaic history.
		uint32_t Index;

		/// Flag indicating whether or not the mosaic is active.
		bool IsActive;
	};
}}}
