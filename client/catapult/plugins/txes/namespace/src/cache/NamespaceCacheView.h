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
#include "NamespaceCacheMixins.h"
#include "NamespaceCacheSerializers.h"
#include "NamespaceCacheTypes.h"
#include "ReadOnlyNamespaceCache.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"

namespace catapult { namespace cache {

	/// Mixins used by the namespace cache view.
	struct NamespaceCacheViewMixins {
	private:
		using PrimaryMixins = PatriciaTreeCacheMixins<NamespaceCacheTypes::PrimaryTypes::BaseSetType, NamespaceCacheDescriptor>;
		using FlatMapMixins = BasicCacheMixins<NamespaceCacheTypes::FlatMapTypes::BaseSetType, NamespaceCacheDescriptor>;

	public:
		using Size = PrimaryMixins::Size;
		using Contains = FlatMapMixins::Contains;
		using Iteration = PrimaryMixins::Iteration;
		using PatriciaTreeView = PrimaryMixins::PatriciaTreeView;

		using NamespaceDeepSize = NamespaceDeepSizeMixin;
		using NamespaceLookup = NamespaceLookupMixin<
			NamespaceCacheTypes::PrimaryTypes::BaseSetType,
			NamespaceCacheTypes::FlatMapTypes::BaseSetType>;
	};

	/// Basic view on top of the namespace cache.
	class BasicNamespaceCacheView
			: public utils::MoveOnly
			, public NamespaceCacheViewMixins::Size
			, public NamespaceCacheViewMixins::Contains
			, public NamespaceCacheViewMixins::Iteration
			, public NamespaceCacheViewMixins::PatriciaTreeView
			, public NamespaceCacheViewMixins::NamespaceDeepSize
			, public NamespaceCacheViewMixins::NamespaceLookup {
	public:
		using ReadOnlyView = NamespaceCacheTypes::CacheReadOnlyType;

	public:
		/// Creates a view around \a namespaceSets, \a options and \a namespaceSizes.
		BasicNamespaceCacheView(
				const NamespaceCacheTypes::BaseSets& namespaceSets,
				const NamespaceCacheTypes::Options& options,
				const NamespaceSizes& namespaceSizes)
				: NamespaceCacheViewMixins::Size(namespaceSets.Primary)
				, NamespaceCacheViewMixins::Contains(namespaceSets.FlatMap)
				, NamespaceCacheViewMixins::Iteration(namespaceSets.Primary)
				, NamespaceCacheViewMixins::PatriciaTreeView(namespaceSets.PatriciaTree.get())
				, NamespaceCacheViewMixins::NamespaceDeepSize(namespaceSizes)
				, NamespaceCacheViewMixins::NamespaceLookup(namespaceSets.Primary, namespaceSets.FlatMap)
				, m_gracePeriodDuration(options.GracePeriodDuration)
		{}

	public:
		/// Gets the grace period duration.
		BlockDuration gracePeriodDuration() const {
			return m_gracePeriodDuration;
		}

	private:
		BlockDuration m_gracePeriodDuration;
	};

	/// View on top of the namespace cache.
	class NamespaceCacheView : public ReadOnlyViewSupplier<BasicNamespaceCacheView> {
	public:
		/// Creates a view around \a namespaceSets, \a options and \a namespaceSizes.
		NamespaceCacheView(
				const NamespaceCacheTypes::BaseSets& namespaceSets,
				const NamespaceCacheTypes::Options& options,
				const NamespaceSizes& namespaceSizes)
				: ReadOnlyViewSupplier(namespaceSets, options, namespaceSizes)
		{}
	};
}}
