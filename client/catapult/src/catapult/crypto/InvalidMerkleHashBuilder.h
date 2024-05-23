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
#include "catapult/types.h"
#include <vector>

namespace catapult {
namespace crypto {

	/// Builder for calculating an insecure, incomplete merkle-like hash.
	/// This replicates an insecure merkle hash algorithm used in the original TS SDK.
	/// It is only for backwards compatability and should not be used in any new code.
	class InvalidMerkleHashBuilder {
	public:
		/// Creates a new merkle hash builder with the specified initial \a capacity.
		explicit InvalidMerkleHashBuilder(size_t capacity = 0);

	public:
		/// Adds \a hash to the merkle hash.
		void update(const Hash256& hash);

		/// Finalizes the merkle hash into \a hash.
		void final(Hash256& hash);

	private:
		std::vector<Hash256> m_hashes;
	};
}
}
