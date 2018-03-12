#pragma once
#include "MultisigCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Basic view on top of the multisig cache.
	class BasicMultisigCacheView
			: public utils::MoveOnly
			, public SizeMixin<MultisigCacheTypes::BaseSetType>
			, public ContainsMixin<MultisigCacheTypes::BaseSetType, MultisigCacheDescriptor>
			, public MapIterationMixin<MultisigCacheTypes::BaseSetType, MultisigCacheDescriptor>
			, public ConstAccessorMixin<MultisigCacheTypes::BaseSetType, MultisigCacheDescriptor> {
	public:
		using ReadOnlyView = MultisigCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a multisigEntries.
		explicit BasicMultisigCacheView(const MultisigCacheTypes::BaseSetType& multisigEntries)
				: SizeMixin<MultisigCacheTypes::BaseSetType>(multisigEntries)
				, ContainsMixin<MultisigCacheTypes::BaseSetType, MultisigCacheDescriptor>(multisigEntries)
				, MapIterationMixin<MultisigCacheTypes::BaseSetType, MultisigCacheDescriptor>(multisigEntries)
				, ConstAccessorMixin<MultisigCacheTypes::BaseSetType, MultisigCacheDescriptor>(multisigEntries)
		{}
	};

	/// View on top of the multisig cache.
	class MultisigCacheView : public ReadOnlyViewSupplier<BasicMultisigCacheView> {
	public:
		/// Creates a view around \a multisigEntries.
		explicit MultisigCacheView(const MultisigCacheTypes::BaseSetType& multisigEntries)
				: ReadOnlyViewSupplier(multisigEntries)
		{}
	};
}}
