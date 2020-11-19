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
#include "BaseSetDefaultTraits.h"
#include "catapult/utils/traits/StlTraits.h"
#include <unordered_set>

namespace catapult { namespace deltaset {

	/// Mixin that wraps BaseSetDelta and provides a facade on top of BaseSetDelta::deltas().
	template<typename TSetDelta>
	class DeltaElementsMixin {
	private:
		// region value accessors

		template<typename TSet, bool IsMap = utils::traits::is_map_v<TSet>>
		struct ValueAccessorT {
			using ValueType = typename TSet::value_type;

			static const ValueType* GetPointer(const typename TSet::value_type& value) {
				return &value;
			}
		};

		template<typename TSet>
		struct ValueAccessorT<TSet, true> { // map specialization
			using ValueType = typename TSet::value_type::second_type;

			static const ValueType* GetPointer(const typename TSet::value_type& pair) {
				return &pair.second;
			}
		};

		// endregion

	private:
		template<typename TSet>
		struct PointerComparer {
			bool operator()(const typename TSet::value_type* pLhs, const typename TSet::value_type* pRhs) const {
				return typename TSet::key_compare()(*pLhs, *pRhs);
			}
		};

		// use MemorySetType for detection because it is always stl (memory) container
		using MemorySetType = typename TSetDelta::MemorySetType;
		using ValueAccessor = ValueAccessorT<MemorySetType>;
		using ValueType = typename ValueAccessor::ValueType;

		// only ordered sets (not maps) support ordered pointers
		// [this constraint is inconsequential because ordered maps aren't currently used]
		using PointerContainer = std::conditional_t<
			!utils::traits::is_map_v<MemorySetType> && utils::traits::is_ordered_v<MemorySetType>,
			std::set<const ValueType*, PointerComparer<MemorySetType>>,
			std::unordered_set<const ValueType*>
		>;

	public:
		/// Creates a mixin around \a setDelta.
		explicit DeltaElementsMixin(const TSetDelta& setDelta) : m_setDelta(setDelta)
		{}

	public:
		/// Gets the pointers to all added elements.
		PointerContainer addedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Added);
		}

		/// Gets the pointers to all modified elements.
		PointerContainer modifiedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Copied);
		}

		/// Gets the pointers to all removed elements.
		PointerContainer removedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Removed);
		}

	private:
		template<typename TSource>
		static PointerContainer CollectAllPointers(const TSource& source) {
			PointerContainer dest;
			for (const auto& value : source)
				dest.insert(ValueAccessor::GetPointer(value));

			return dest;
		}

	private:
		const TSetDelta& m_setDelta;
	};
}}
