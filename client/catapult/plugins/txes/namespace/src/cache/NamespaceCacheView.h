#pragma once
#include "NamespaceCacheMixins.h"
#include "NamespaceCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the namespace cache view.
	struct NamespaceCacheViewMixins {
		using Size = SizeMixin<NamespaceCacheTypes::PrimaryTypes::BaseSetType>;
		using Contains = ContainsMixin<NamespaceCacheTypes::FlatMapTypes::BaseSetType, NamespaceCacheDescriptor>;
		using MapIteration = MapIterationMixin<NamespaceCacheTypes::PrimaryTypes::BaseSetType, NamespaceCacheDescriptor>;

		using NamespaceDeepSize = NamespaceDeepSizeMixin<NamespaceCacheTypes::PrimaryTypes::BaseSetType>;
		using NamespaceLookup = NamespaceLookupMixin<
			NamespaceCacheTypes::PrimaryTypes::BaseSetType,
			NamespaceCacheTypes::FlatMapTypes::BaseSetType>;
	};

	/// Basic view on top of the namespace cache.
	class BasicNamespaceCacheView
			: public utils::MoveOnly
			, public NamespaceCacheViewMixins::Size
			, public NamespaceCacheViewMixins::Contains
			, public NamespaceCacheViewMixins::MapIteration
			, public NamespaceCacheViewMixins::NamespaceDeepSize
			, public NamespaceCacheViewMixins::NamespaceLookup {
	public:
		using ReadOnlyView = NamespaceCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a namespaceSets.
		explicit BasicNamespaceCacheView(const NamespaceCacheTypes::BaseSetType& namespaceSets)
				: NamespaceCacheViewMixins::Size(namespaceSets.Primary)
				, NamespaceCacheViewMixins::Contains(namespaceSets.FlatMap)
				, NamespaceCacheViewMixins::MapIteration(namespaceSets.Primary)
				, NamespaceCacheViewMixins::NamespaceDeepSize(namespaceSets.Primary)
				, NamespaceCacheViewMixins::NamespaceLookup(namespaceSets.Primary, namespaceSets.FlatMap)
		{}
	};

	/// View on top of the namespace cache.
	class NamespaceCacheView : public ReadOnlyViewSupplier<BasicNamespaceCacheView> {
	public:
		/// Creates a view around \a namespaceSets.
		explicit NamespaceCacheView(const NamespaceCacheTypes::BaseSetType& namespaceSets)
				: ReadOnlyViewSupplier(namespaceSets)
		{}
	};
}}
