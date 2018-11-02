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
#include "src/cache/PropertyCache.h"
#include "catapult/validators/ValidationResult.h"
#include "catapult/types.h"

namespace catapult { namespace cache { class ReadOnlyCatapultCache; } }

namespace catapult { namespace validators {

	/// A view on top of a catapult cache cache for retrieving a typed account property.
	class AccountPropertyView {
	public:
		/// Creates a view around \a cache.
		explicit AccountPropertyView(const cache::ReadOnlyCatapultCache& cache);

	public:
		/// Tries to initialize the internal iterator for account properties with \a address.
		bool initialize(const Address& address);

		/// Gets the typed account property specified by \a propertyType.
		/// \throws catapult_invalid_argument if the view does not point to a value.
		template<typename TPropertyValue>
		state::TypedAccountProperty<TPropertyValue> get(model::PropertyType propertyType) const {
			const auto& accountProperties = m_iter.get();
			return accountProperties.template property<TPropertyValue>(propertyType);
		}

		/// Returns \c true if \a value is allowed.
		template<typename TPropertyValue>
		bool isAllowed(model::PropertyType propertyType, const TPropertyValue& value) {
			auto typedProperty = get<TPropertyValue>(propertyType);
			if (0 == typedProperty.size())
				return true;

			const auto& descriptor = typedProperty.descriptor();
			if (state::OperationType::Allow == descriptor.operationType())
				return typedProperty.contains(value);
			else
				return !typedProperty.contains(value);
		}

	private:
		using FindIterator = cache::PropertyCacheTypes::CacheReadOnlyType::ReadOnlyFindIterator<
			cache::PropertyCacheView::const_iterator,
			cache::PropertyCacheDelta::const_iterator
		>;

		const cache::ReadOnlyCatapultCache& m_cache;
		FindIterator m_iter;
	};
}}
