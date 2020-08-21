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
#include "catapult/model/ContiguousEntityContainer.h"
#include <type_traits>

namespace catapult { namespace test {

	/// Gets the number of entities in \a container.
	template<typename TEntity>
	size_t CountContainerEntities(model::BasicContiguousEntityContainer<TEntity>&& container) {
		size_t count = 0;
		for (auto iter = container.begin(); container.end() != iter; ++iter)
			++count;

		return count;
	}

	/// Traits for accessing a source via a const reference.
	template<typename TSource>
	struct ConstTraitsT {
		using SourceType = std::add_const_t<TSource>;

		/// Gets a const reference to \a source.
		static SourceType& GetAccessor(SourceType& source) {
			return source;
		}
	};

	/// Traits for accessing a source via a non-const reference.
	template<typename TSource>
	struct NonConstTraitsT {
		using SourceType = std::remove_const_t<TSource>;

		/// Gets a non-const reference to \a source.
		static SourceType& GetAccessor(SourceType& source) {
			return source;
		}
	};
}}
