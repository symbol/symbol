#pragma once
#include "catapult/model/TransactionPlugin.h"

namespace catapult { namespace mocks {

	/// An offset buffer range.
	struct OffsetRange {
		/// The start offset (inclusive).
		size_t Start;
		/// The end offset (exclusive).
		size_t End;
	};

	/// Extracts a raw buffer \a range from \a pVoid.
	RawBuffer ExtractBuffer(const OffsetRange& range, const void* pVoid);

	/// Creates a (mock) transaction plugin with custom buffers around \a dataRange and \a supplementalRanges.
	std::unique_ptr<model::TransactionPlugin> CreateMockTransactionPluginWithCustomBuffers(
			const OffsetRange& dataRange,
			const std::vector<OffsetRange>& supplementalRanges);
}}
