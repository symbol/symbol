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
#include "CacheMixins.h"
#include "PatriciaTreeCacheMixins.h"
#include "catapult/deltaset/DeltaElementsMixin.h"

namespace catapult { namespace cache {

	/// Grouping of all basic cache mixins for a single set.
	template<typename TSet, typename TCacheDescriptor>
	struct BasicCacheMixins {
		using Size = SizeMixin<TSet>;
		using Contains = ContainsMixin<TSet, TCacheDescriptor>;
		using Iteration = IterationMixin<TSet>;

		using ConstAccessor = ConstAccessorMixin<TSet, TCacheDescriptor>;
		using MutableAccessor = MutableAccessorMixin<TSet, TCacheDescriptor>;

		template<typename TValueAdapter>
		using ConstAccessorWithAdapter = ConstAccessorMixin<TSet, TCacheDescriptor, TValueAdapter>;

		template<typename TValueAdapter>
		using MutableAccessorWithAdapter = MutableAccessorMixin<TSet, TCacheDescriptor, TValueAdapter>;

		using ActivePredicate = ActivePredicateMixin<TSet, TCacheDescriptor>;
		using BasicInsertRemove = BasicInsertRemoveMixin<TSet, TCacheDescriptor>;

		using DeltaElements = deltaset::DeltaElementsMixin<TSet>;
	};

	/// Grouping of all basic and patricia tree cache mixins for a single set.
	template<typename TSet, typename TCacheDescriptor>
	struct PatriciaTreeCacheMixins : public BasicCacheMixins<TSet, TCacheDescriptor> {
		using PatriciaTreeView = PatriciaTreeMixin<typename TCacheDescriptor::PatriciaTree>;
		using PatriciaTreeDelta = PatriciaTreeDeltaMixin<TSet, typename TCacheDescriptor::PatriciaTree::DeltaType>;
	};
}}
