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
#include "src/state/NamespaceEntry.h"
#include "src/state/RootNamespaceHistory.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include <numeric>

namespace catapult { namespace cache {

	/// Extended namespace sizes.
	struct NamespaceSizes {
		/// Number of unique active namespaces.
		size_t Active;

		/// Total number of namespaces (including all versions).
		size_t Deep;
	};

	/// Mixin for calculating the deep size of namespaces.
	class NamespaceDeepSizeMixin {
	public:
		/// Creates a mixin around \a sizes.
		explicit NamespaceDeepSizeMixin(const NamespaceSizes& sizes) : m_sizes(sizes)
		{}

	public:
		/// Gets the number of unique active namespaces in the cache.
		size_t activeSize() const {
			return m_sizes.Active;
		}

		/// Gets the total number of namespaces in the cache (including versions).
		size_t deepSize() const {
			return m_sizes.Deep;
		}

	protected:
		/// Increments the active size by \a delta.
		void incrementActiveSize(size_t delta = 1) {
			m_sizes.Active += delta;
		}

		/// Decrements the active size by \a delta.
		void decrementActiveSize(size_t delta = 1) {
			m_sizes.Active -= delta;
		}

		/// Increments the deep size by \a delta.
		void incrementDeepSize(size_t delta = 1) {
			m_sizes.Deep += delta;
		}

		/// Decrements the deep size by \a delta.
		void decrementDeepSize(size_t delta = 1) {
			m_sizes.Deep -= delta;
		}

	private:
		NamespaceSizes m_sizes;
	};

	/// Mixin for looking up namespaces.
	/// \note Due to double lookups, this cannot be replaced with typical ActivePredicateMixin and ConstPredicateMixin.
	template<typename TPrimarySet, typename TFlatMap>
	class NamespaceLookupMixin {
	public:
		/// Iterator that is returned by namespace cache find functions.
		class const_iterator {
		public:
			/// Creates an uninitialized iterator.
			const_iterator() : m_isValid(false)
			{}

			/// Creates an uninitialized iterator around \a id.
			explicit const_iterator(NamespaceId id)
					: m_isValid(false)
					, m_id(id)
			{}

			/// Creates an iterator around a namespace iterator (\a namespaceIter) and a root iterator (\a rootIter).
			const_iterator(typename TFlatMap::FindConstIterator&& namespaceIter, typename TPrimarySet::FindConstIterator&& rootIter)
					: m_isValid(true)
					, m_namespaceIter(std::move(namespaceIter))
					, m_rootIter(std::move(rootIter))
					, m_entry(*m_namespaceIter.get(), m_rootIter.get()->back())
			{}

		public:
			/// Gets a const value.
			/// \throws catapult_invalid_argument if this iterator does not point to a value.
			const state::NamespaceEntry& get() const {
				if (!m_isValid)
					CATAPULT_THROW_INVALID_ARGUMENT_1("unknown namespace", m_id);

				return *tryGet();
			}

			/// Tries to get a const value.
			const state::NamespaceEntry* tryGet() const {
				return m_isValid ? &m_entry : nullptr;
			}

			/// Tries to get a const (unadapted) value.
			const state::RootNamespaceHistory* tryGetUnadapted() const {
				return m_rootIter.get();
			}

		private:
			bool m_isValid;
			NamespaceId m_id;
			typename TFlatMap::FindConstIterator m_namespaceIter;
			typename TPrimarySet::FindConstIterator m_rootIter;
			state::NamespaceEntry m_entry;
		};

	public:
		/// Creates a mixin around (history by id) \a set and \a flatMap.
		NamespaceLookupMixin(const TPrimarySet& set, const TFlatMap& flatMap)
				: m_set(set)
				, m_flatMap(flatMap)
		{}

	public:
		/// Finds the cache value identified by \a id.
		const_iterator find(NamespaceId id) const {
			auto namespaceIter = m_flatMap.find(id);
			if (!namespaceIter.get())
				return const_iterator(id);

			auto rootIter = m_set.find(namespaceIter.get()->rootId());
			return const_iterator(std::move(namespaceIter), std::move(rootIter));
		}

	private:
		const TPrimarySet& m_set;
		const TFlatMap& m_flatMap;
	};
}}
