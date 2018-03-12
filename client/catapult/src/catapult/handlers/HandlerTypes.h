#pragma once
#include "catapult/model/AnnotatedEntityRange.h"
#include "catapult/functions.h"

namespace catapult { namespace handlers {

	/// A handler for processing an annotated entity range.
	template<typename TEntity>
	using RangeHandler = consumer<model::AnnotatedEntityRange<TEntity>&&>;

	/// Prototype for a function that processes a range of blocks.
	using BlockRangeHandler = RangeHandler<model::Block>;

	/// Prototype for a function that processes a range of transactions.
	using TransactionRangeHandler = RangeHandler<model::Transaction>;
}}
