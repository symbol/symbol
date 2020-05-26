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

#include "Validators.h"
#include "ActiveMosaicView.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"

namespace catapult { namespace validators {

	ActiveMosaicView::ActiveMosaicView(const cache::ReadOnlyCatapultCache& cache) : m_cache(cache)
	{}

	validators::ValidationResult ActiveMosaicView::tryGet(MosaicId id, Height height, FindIterator& iter) const {
		// ensure that the mosaic is active
		const auto& mosaicCache = m_cache.sub<cache::MosaicCache>();
		iter = mosaicCache.find(id);
		return !iter.tryGet() || !iter.get().definition().isActive(height)
				? Failure_Mosaic_Expired
				: ValidationResult::Success;
	}

	validators::ValidationResult ActiveMosaicView::tryGet(MosaicId id, Height height, const Address& owner, FindIterator& iter) const {
		auto result = tryGet(id, height, iter);
		if (!IsValidationResultSuccess(result))
			return result;

		return iter.get().definition().ownerAddress() != owner ? Failure_Mosaic_Owner_Conflict : ValidationResult::Success;
	}
}}
