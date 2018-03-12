#pragma once
#include "MultisigCacheTypes.h"
#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/deltaset/DeltaElementsMixin.h"

namespace catapult { namespace cache {

	/// Basic delta on top of the multisig cache.
	class BasicMultisigCacheDelta
			: public utils::MoveOnly
			, public SizeMixin<MultisigCacheTypes::BaseSetDeltaType>
			, public ContainsMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>
			, public ConstAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>
			, public MutableAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>
			, public BasicInsertRemoveMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>
			, public deltaset::DeltaElementsMixin<MultisigCacheTypes::BaseSetDeltaType> {
	public:
		using ReadOnlyView = MultisigCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a delta based on the multisig entries map (\a pMultisigEntries).
		explicit BasicMultisigCacheDelta(const MultisigCacheTypes::BaseSetDeltaPointerType& pMultisigEntries)
				: SizeMixin<MultisigCacheTypes::BaseSetDeltaType>(*pMultisigEntries)
				, ContainsMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>(*pMultisigEntries)
				, ConstAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>(*pMultisigEntries)
				, MutableAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>(*pMultisigEntries)
				, BasicInsertRemoveMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>(*pMultisigEntries)
				, deltaset::DeltaElementsMixin<MultisigCacheTypes::BaseSetDeltaType>(*pMultisigEntries)
				, m_pMultisigEntries(pMultisigEntries)
		{}

	public:
		using ConstAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>::get;
		using MutableAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>::get;

		using ConstAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>::tryGet;
		using MutableAccessorMixin<MultisigCacheTypes::BaseSetDeltaType, MultisigCacheDescriptor>::tryGet;

	private:
		MultisigCacheTypes::BaseSetDeltaPointerType m_pMultisigEntries;
	};

	/// Delta on top of the multisig cache.
	class MultisigCacheDelta : public ReadOnlyViewSupplier<BasicMultisigCacheDelta> {
	public:
		/// Creates a delta around \a pMultisigEntries.
		explicit MultisigCacheDelta(const MultisigCacheTypes::BaseSetDeltaPointerType& pMultisigEntries)
				: ReadOnlyViewSupplier(pMultisigEntries)
		{}
	};
}}
