#pragma once
#include "catapult/model/Elements.h"

namespace catapult { namespace io {

	/// Block change subscriber.
	class BlockChangeSubscriber {
	public:
		virtual ~BlockChangeSubscriber() {}

	public:
		/// Indicates \a blockElement was saved.
		virtual void notifyBlock(const model::BlockElement& blockElement) = 0;

		/// Indicates all blocks after \a height were invalidated.
		virtual void notifyDropBlocksAfter(Height height) = 0;
	};
}}
