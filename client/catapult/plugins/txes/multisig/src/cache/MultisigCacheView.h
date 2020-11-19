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
#include "MultisigBaseSets.h"
#include "MultisigCacheSerializers.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyArtifactCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the multisig cache view.
	using MultisigCacheViewMixins = PatriciaTreeCacheMixins<MultisigCacheTypes::PrimaryTypes::BaseSetType, MultisigCacheDescriptor>;

	/// Basic view on top of the multisig cache.
	class BasicMultisigCacheView
			: public utils::MoveOnly
			, public MultisigCacheViewMixins::Size
			, public MultisigCacheViewMixins::Contains
			, public MultisigCacheViewMixins::Iteration
			, public MultisigCacheViewMixins::ConstAccessor
			, public MultisigCacheViewMixins::PatriciaTreeView {
	public:
		using ReadOnlyView = MultisigCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a multisigSets.
		explicit BasicMultisigCacheView(const MultisigCacheTypes::BaseSets& multisigSets)
				: MultisigCacheViewMixins::Size(multisigSets.Primary)
				, MultisigCacheViewMixins::Contains(multisigSets.Primary)
				, MultisigCacheViewMixins::Iteration(multisigSets.Primary)
				, MultisigCacheViewMixins::ConstAccessor(multisigSets.Primary)
				, MultisigCacheViewMixins::PatriciaTreeView(multisigSets.PatriciaTree.get())
		{}
	};

	/// View on top of the multisig cache.
	class MultisigCacheView : public ReadOnlyViewSupplier<BasicMultisigCacheView> {
	public:
		/// Creates a view around \a multisigSets.
		explicit MultisigCacheView(const MultisigCacheTypes::BaseSets& multisigSets)
				: ReadOnlyViewSupplier(multisigSets)
		{}
	};
}}
