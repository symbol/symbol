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
