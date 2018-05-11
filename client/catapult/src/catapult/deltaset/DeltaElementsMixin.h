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
#include "BaseSetDefaultTraits.h"
#include <unordered_set>

namespace catapult { namespace deltaset {

	/// Mixin that wraps BaseSetDelta and provides a facade on top of BaseSetDelta::deltas().
	/// \note This only works for map-based delta sets.
	template<typename TSetDelta>
	class DeltaElementsMixin {
	private:
		// used to dereference values and values pointed to by shared_ptr
		// (this is required to support shared_ptr value types in BaseSet)

		template<typename TElement>
		struct DerefHelperT {
			using const_pointer_type = const TElement*;

			static const TElement& Deref(const TElement& element) {
				return element;
			}
		};

		template<typename T>
		struct DerefHelperT<std::shared_ptr<T>> {
			using const_pointer_type = const T*;

			static const T& Deref(const std::shared_ptr<T>& element) {
				return *element;
			}
		};

	private:
		using DerefHelper = DerefHelperT<typename TSetDelta::SetType::value_type::second_type>;
		using PointerContainer = std::unordered_set<typename DerefHelper::const_pointer_type>;

	public:
		/// Creates a mixin around \a setDelta.
		explicit DeltaElementsMixin(const TSetDelta& setDelta) : m_setDelta(setDelta)
		{}

	public:
		/// Gets pointers to all added elements.
		auto addedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Added);
		}

		/// Gets pointers to all modified elements.
		auto modifiedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Copied);
		}

		/// Gets pointers to all removed elements.
		auto removedElements() const {
			return CollectAllPointers(m_setDelta.deltas().Removed);
		}

	private:
		template<typename TSource>
		static PointerContainer CollectAllPointers(const TSource& source) {
			PointerContainer dest;
			for (const auto& pair : source)
				dest.insert(&DerefHelper::Deref(pair.second));

			return dest;
		}

	private:
		const TSetDelta& m_setDelta;
	};
}}
