/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "src/cache/AccountRestrictionCache.h"
#include "src/state/AccountRestrictionUtils.h"
#include "catapult/validators/ValidationResult.h"
#include "catapult/types.h"

namespace catapult { namespace cache { class ReadOnlyCatapultCache; } }

namespace catapult { namespace validators {

	/// View on top of a catapult cache cache for retrieving a typed account restriction.
	class AccountRestrictionView {
	public:
		/// Creates a view around \a cache.
		explicit AccountRestrictionView(const cache::ReadOnlyCatapultCache& cache);

	public:
		/// Tries to initialize the internal iterator for account restriction with \a address.
		bool initialize(const Address& address);

		/// Gets the typed account restriction specified by \a restrictionFlags.
		/// \throws catapult_invalid_argument if the view does not point to a value.
		const state::AccountRestriction& get(model::AccountRestrictionFlags restrictionFlags) const {
			const auto& restrictions = m_iter.get();
			return restrictions.restriction(restrictionFlags);
		}

		/// Returns \c true if \a value is allowed.
		template<typename TRestrictionValue>
		bool isAllowed(model::AccountRestrictionFlags restrictionFlags, const TRestrictionValue& value) {
			const auto& restriction = get(restrictionFlags);
			if (0 == restriction.values().size())
				return true;

			const auto& descriptor = restriction.descriptor();
			if (state::AccountRestrictionOperationType::Allow == descriptor.operationType())
				return restriction.contains(state::ToVector(value));
			else
				return !restriction.contains(state::ToVector(value));
		}

	private:
		using FindIterator = cache::AccountRestrictionCacheTypes::CacheReadOnlyType::ReadOnlyFindIterator<
			cache::AccountRestrictionCacheView::const_iterator,
			cache::AccountRestrictionCacheDelta::const_iterator
		>;

		const cache::ReadOnlyCatapultCache& m_cache;
		FindIterator m_iter;
	};
}}
