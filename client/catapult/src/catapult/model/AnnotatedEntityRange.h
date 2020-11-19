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
#include "Block.h"
#include "EntityRange.h"
#include "NodeIdentity.h"
#include "Transaction.h"

namespace catapult { namespace model {

	/// Combination of an entity range and optional context.
	template<typename TEntity>
	struct AnnotatedEntityRange {
	public:
		/// Creates a default annotated entity range.
		AnnotatedEntityRange() = default;

		/// Creates an annotated entity range around \a range without context.
		AnnotatedEntityRange(EntityRange<TEntity>&& range) : AnnotatedEntityRange(std::move(range), NodeIdentity())
		{}

		/// Creates an annotated entity range around \a range and a source identity (\a sourceIdentity).
		AnnotatedEntityRange(EntityRange<TEntity>&& range, const NodeIdentity& sourceIdentity)
				: Range(std::move(range))
				, SourceIdentity(sourceIdentity)
		{}

	public:
		/// Entity range.
		EntityRange<TEntity> Range;

		/// Source identity (optional).
		NodeIdentity SourceIdentity;
	};

	/// Annotated entity range composed of blocks.
	using AnnotatedBlockRange = AnnotatedEntityRange<Block>;

	/// Annotated entity range composed of transactions.
	using AnnotatedTransactionRange = AnnotatedEntityRange<Transaction>;
}}
