#pragma once
#include "NamespaceCacheMixins.h"
#include "NamespaceCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/DeltaElementsMixin.h"

namespace catapult { namespace cache {

	/// Mixins used by the namespace delta view.
	struct NamespaceCacheDeltaMixins {
		using Size = SizeMixin<NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaType>;
		using Contains = ContainsMixin<NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaType, NamespaceCacheDescriptor>;
		using DeltaElements = deltaset::DeltaElementsMixin<NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaType>;

		using NamespaceDeepSize = NamespaceDeepSizeMixin<NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaType>;
		using NamespaceLookup = NamespaceLookupMixin<
			NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaType,
			NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaType>;
	};

	/// Basic delta on top of the namespace cache.
	class BasicNamespaceCacheDelta
			: public utils::MoveOnly
			, public NamespaceCacheDeltaMixins::Size
			, public NamespaceCacheDeltaMixins::Contains
			, public NamespaceCacheDeltaMixins::DeltaElements
			, public NamespaceCacheDeltaMixins::NamespaceDeepSize
			, public NamespaceCacheDeltaMixins::NamespaceLookup {
	public:
		using ReadOnlyView = NamespaceCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta around \a namespaceSets.
		explicit BasicNamespaceCacheDelta(const NamespaceCacheTypes::BaseSetDeltaPointerType& namespaceSets);

	public:
		/// Inserts the root namespace \a ns into the cache.
		void insert(const state::RootNamespace& ns);

		/// Inserts the namespace \a ns into the cache.
		void insert(const state::Namespace& ns);

		/// Removes the namespace specified by its \a id from the cache.
		void remove(NamespaceId id);

		/// Prunes the namespace cache at \a height.
		void prune(Height height);

	private:
		void removeRoot(NamespaceId id);
		void removeChild(const state::Namespace& ns);

	private:
		NamespaceCacheTypes::NamespaceCacheTypes::FlatMapTypes::BaseSetDeltaPointerType m_pNamespaceById;
		NamespaceCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pHistoryById;
	};

	/// Delta on top of the namespace cache.
	class NamespaceCacheDelta : public ReadOnlyViewSupplier<BasicNamespaceCacheDelta> {
	public:
		/// Creates a delta around \a namespaceSets.
		explicit NamespaceCacheDelta(const NamespaceCacheTypes::BaseSetDeltaPointerType& namespaceSets)
				: ReadOnlyViewSupplier(namespaceSets)
		{}
	};
}}
