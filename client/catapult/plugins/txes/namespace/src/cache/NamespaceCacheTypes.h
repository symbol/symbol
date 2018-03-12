#pragma once
#include "src/state/Namespace.h"
#include "src/state/NamespaceEntry.h"
#include "src/state/RootNamespaceHistory.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		class BasicNamespaceCacheDelta;
		class BasicNamespaceCacheView;
		class NamespaceCache;
		class NamespaceCacheDelta;
		class NamespaceCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
		class ReadOnlyArtifactCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a namespace cache.
	struct NamespaceCacheDescriptor {
	public:
		// key value types
		using KeyType = NamespaceId;
		using ValueType = state::RootNamespaceHistory;

		// cache types
		using CacheType = NamespaceCache;
		using CacheDeltaType = NamespaceCacheDelta;
		using CacheViewType = NamespaceCacheView;

	public:
		/// Gets the key corresponding to \a history.
		static auto GetKeyFromValue(const ValueType& history) {
			return history.id();
		}
	};

	/// Namespace cache types.
	struct NamespaceCacheTypes {
	public:
		using CacheReadOnlyType = ReadOnlyArtifactCache<
			BasicNamespaceCacheView,
			BasicNamespaceCacheDelta,
			NamespaceId,
			state::NamespaceEntry>;

	// region secondary descriptors

	private:
		struct FlatMapTypesDescriptor {
		public:
			using KeyType = NamespaceId;
			using ValueType = state::Namespace;

		public:
			static auto GetKeyFromValue(const ValueType& ns) {
				return ns.id();
			}
		};

	// endregion

	public:
		using PrimaryTypes = MutableUnorderedMapAdapter<NamespaceCacheDescriptor, utils::BaseValueHasher<NamespaceId>>;
		using FlatMapTypes = MutableUnorderedMapAdapter<FlatMapTypesDescriptor, utils::BaseValueHasher<NamespaceId>>;

	public:
		// in order to compose namespace cache from multiple sets, define an aggregate set type

		struct BaseSetDeltaPointerType {
			PrimaryTypes::BaseSetDeltaPointerType pPrimary;
			FlatMapTypes::BaseSetDeltaPointerType pFlatMap;
		};

		struct BaseSetType {
		public:
			PrimaryTypes::BaseSetType Primary;
			FlatMapTypes::BaseSetType FlatMap;

		public:
			BaseSetDeltaPointerType rebase() {
				return { Primary.rebase(), FlatMap.rebase() };
			}

			BaseSetDeltaPointerType rebaseDetached() const {
				return { Primary.rebaseDetached(), FlatMap.rebaseDetached() };
			}

			void commit() {
				Primary.commit();
				FlatMap.commit();
			}
		};
	};
}}
