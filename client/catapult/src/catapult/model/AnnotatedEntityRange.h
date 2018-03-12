#pragma once
#include "Block.h"
#include "EntityRange.h"
#include "Transaction.h"

namespace catapult { namespace model {

	/// Combination of an entity range and optional context.
	template<typename TEntity>
	struct AnnotatedEntityRange {
	public:
		/// Creates a default annotated entity range.
		AnnotatedEntityRange() = default;

		/// Creates an annotated entity range around \a range without context.
		AnnotatedEntityRange(EntityRange<TEntity>&& range) : AnnotatedEntityRange(std::move(range), {})
		{}

		/// Creates an annotated entity range around \a range and a source public key (\a sourcePublicKey).
		AnnotatedEntityRange(EntityRange<TEntity>&& range, const Key& sourcePublicKey)
				: Range(std::move(range))
				, SourcePublicKey(sourcePublicKey)
		{}

	public:
		/// The entity range.
		EntityRange<TEntity> Range;

		/// The (optional) source public key.
		Key SourcePublicKey;
	};

	/// An annotated entity range composed of blocks.
	using AnnotatedBlockRange = AnnotatedEntityRange<Block>;

	/// An annotated entity range composed of transactions.
	using AnnotatedTransactionRange = AnnotatedEntityRange<Transaction>;
}}
