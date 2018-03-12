#pragma once
#include "SynchronizedCache.h"
#include "catapult/utils/Casting.h"

namespace catapult { namespace cache {

	/// Cache ids for well-known caches.
	enum class CacheId : uint32_t {
		AccountState,
		BlockDifficulty,
		Hash,
		Namespace,
		Mosaic,
		Multisig,
		HashLockInfo,
		SecretLockInfo
	};

	/// Basic cache implementation that supports multiple views and committing.
	/// \note Typically, TSubViewArgs will expand to zero or one types.
	template<typename TCacheDescriptor, typename TBaseSet, typename... TSubViewArgs>
	class BasicCache : public utils::MoveOnly {
	public:
		using CacheViewType = typename TCacheDescriptor::CacheViewType;
		using CacheDeltaType = typename TCacheDescriptor::CacheDeltaType;
		using CacheReadOnlyType = typename CacheViewType::ReadOnlyView;

	public:
		/// Creates an empty cache with arguments (\a subViewArgs).
		BasicCache(TSubViewArgs&&... subViewArgs) : m_subViewArgs(std::forward<TSubViewArgs>(subViewArgs)...)
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
		// use presence of OrderedSet::Is_Ordered property to determine BaseSet container ordering
		enum class ContainerOrderingType { Ordered, Unordered };
		using OrderedContainerType = std::integral_constant<ContainerOrderingType, ContainerOrderingType::Ordered>;
		using UnorderedContainerType = std::integral_constant<ContainerOrderingType, ContainerOrderingType::Unordered>;

		template<typename TContainer, typename = void>
		struct ContainerPolicy
				: UnorderedContainerType
		{};

		template<typename TContainer>
		struct ContainerPolicy<TContainer, typename utils::traits::enable_if_type<decltype(TContainer::Is_Ordered)>::type>
				: OrderedContainerType
		{};

	private:
		static void Commit(TBaseSet& m_set, const CacheDeltaType&, UnorderedContainerType) {
			m_set.commit();
		}

		static void Commit(TBaseSet& m_set, const CacheDeltaType& delta, OrderedContainerType) {
			m_set.commit(delta.pruningBoundary());
		}

	private:
		TBaseSet m_set;
		std::tuple<TSubViewArgs...> m_subViewArgs;
	};

	/// Defines cache constants for a cache with \a NAME.
#define DEFINE_CACHE_CONSTANTS(NAME) \
	static constexpr size_t Id = utils::to_underlying_type(CacheId::NAME); \
	static constexpr auto Name = #NAME "Cache";
}}
