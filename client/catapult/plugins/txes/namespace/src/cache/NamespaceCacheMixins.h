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

	/// A mixin for calculating the deep size of namespaces.
	template<typename TSet>
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

	/// A mixin for looking up namespaces.
	/// \note Due to double lookups, this cannot be replaced with typical ActivePredicateMixin and ConstPredicateMixin.
	template<typename TPrimarySet, typename TFlatMap>
	class NamespaceLookupMixin {
	public:
		/// Creates a mixin around (history by id) \a set and \a flatMap.
		explicit NamespaceLookupMixin(const TPrimarySet& set, const TFlatMap& flatMap)
				: m_set(set)
				, m_flatMap(flatMap)
		{}

	public:
		/// Returns \c true if the value specified by identifier \a id is active at \a height.
		bool isActive(NamespaceId id, Height height) const {
			return m_flatMap.contains(id) && getHistory(id).back().lifetime().isActive(height);
		}

		/// Gets a value specified by identifier \a id.
		/// \throws catapult_invalid_argument if the requested value is not found.
		state::NamespaceEntry get(NamespaceId id) const {
			const auto& ns = getNamespace(id);
			const auto& root = m_set.find(ns.rootId())->back();
			return state::NamespaceEntry(ns, root);
		}

	private:
		const state::Namespace& getNamespace(NamespaceId id) const {
			const auto* pNamespace = m_flatMap.find(id);
			if (!pNamespace)
				CATAPULT_THROW_INVALID_ARGUMENT_1("unknown namespace", id);

			return *pNamespace;
		}

		const state::RootNamespaceHistory& getHistory(NamespaceId id) const {
			const auto& ns = getNamespace(id);
			const auto* pHistory = m_set.find(ns.rootId());
			if (!pHistory)
				CATAPULT_THROW_RUNTIME_ERROR_1("no history for root namespace found", ns.rootId());

			return *pHistory;
		}

	private:
		const TPrimarySet& m_set;
		const TFlatMap& m_flatMap;
	};
}}
