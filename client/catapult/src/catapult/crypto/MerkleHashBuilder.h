#pragma once
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace crypto {

	/// Builder for creating a merkle hash.
	class MerkleHashBuilder {
	public:
		/// Creates a new merkle hash builder with the specified initial \a capacity.
		explicit MerkleHashBuilder(size_t capacity = 0);

	public:
		/// Adds \a hash to the merkle hash.
		void update(const Hash256& hash);

		/// Finalizes the merkle hash into \a hash.
		void final(Hash256& hash);

	private:
		std::vector<Hash256> m_hashes;
	};
}}
