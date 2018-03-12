#pragma once
#include "catapult/model/RangeTypes.h"
#include <vector>

namespace catapult { namespace test {

	/// Generates a vector of offsets for a given nunber of hashes (\a numHashes).
	std::vector<size_t> GenerateOffsets(size_t numHashes);

	/// Generates \a numHashes random hashes.
	model::HashRange GenerateRandomHashes(size_t numHashes);

	/// Generates a subset of hashes consisting of the first \a numHashes hashes in \a source.
	model::HashRange GenerateRandomHashesSubset(const model::HashRange& source, size_t numHashes);

	/// Inserts all hashes from \a source into the destination vector (\a dest).
	void InsertAll(std::vector<Hash256>& dest, const model::HashRange& source);

	/// Concatenates two entity ranges specified by \a lhs and \a rhs.
	model::HashRange ConcatHashes(const model::HashRange& lhs, const model::HashRange& rhs);
}}
