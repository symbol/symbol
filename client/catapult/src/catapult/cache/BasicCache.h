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
#include "CacheConfiguration.h"
#include "CacheConstants.h"
#include "SynchronizedCache.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace cache {

	/// Basic cache implementation that supports multiple views and committing.
	/// \note Typically, TSubViewArgs will expand to zero or one types.
	template<typename TCacheDescriptor, typename TBaseSet, typename... TSubViewArgs>
	class BasicCache : public utils::MoveOnly {
	public:
		using CacheViewType = typename TCacheDescriptor::CacheViewType;
		using CacheDeltaType = typename TCacheDescriptor::CacheDeltaType;
		using CacheReadOnlyType = typename CacheViewType::ReadOnlyView;

	public:
		/// Creates an empty cache with \a config and arguments (\a subViewArgs).
		BasicCache(const CacheConfiguration& config, TSubViewArgs&&... subViewArgs)
				: m_set(config)
				, m_subViewArgs(std::forward<TSubViewArgs>(subViewArgs)...)
		{}

	public:
		/// Returns a locked view based on this cache.
		CacheViewType createView() const {
			return createSubView<CacheViewType>(m_set);
		}

		/// Returns a locked cache delta based on this cache.
		CacheDeltaType createDelta() {
			return createSubView<CacheDeltaType>(m_set.rebase());
		}

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		CacheDeltaType createDetachedDelta() const {
			return createSubView<CacheDeltaType>(m_set.rebaseDetached());
		}

		/// Commits all pending changes from \a delta to the underlying storage.
		void commit(const CacheDeltaType& delta) {
			Commit(m_set, delta, ContainerPolicy<TBaseSet>());
		}

	private:
		template<typename TView, typename TSetView>
		TView createSubView(const TSetView& setView) const {
			return TView(setView, std::get<TSubViewArgs>(m_subViewArgs)...);
		}

	private:
		template<typename T>
		struct ContainerOrderedFlag : public T
		{};

		template<typename TContainer, typename = void>
		struct ContainerPolicy
				: ContainerOrderedFlag<std::false_type>
		{};

		template<typename TContainer>
		struct ContainerPolicy<TContainer, typename utils::traits::enable_if_type<decltype(TContainer::IsOrderedSet::value)>::type>
				: ContainerOrderedFlag<typename TContainer::IsOrderedSet>
		{};

	private:
		static void Commit(TBaseSet& m_set, const CacheDeltaType&, ContainerOrderedFlag<std::false_type>) {
			m_set.commit();
		}

		static void Commit(TBaseSet& m_set, const CacheDeltaType& delta, ContainerOrderedFlag<std::true_type>) {
			m_set.commit(delta.pruningBoundary());
		}

	private:
		TBaseSet m_set;
		std::tuple<TSubViewArgs...> m_subViewArgs;
	};
}}
