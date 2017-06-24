#pragma once
#include "Conversions.h"

namespace catapult { namespace test {

	/// Gets the nemesis generation hash.
	CATAPULT_INLINE
	Hash256 GetNemesisGenerationHash() {
#ifdef NIS1_COMPATIBLE_SIGNATURES
		constexpr auto Nemesis_Generation_Hash_String = "16ed3d69d3ca67132aace4405aa122e5e041e58741a4364255b15201f5aaf6e4";
#else
		constexpr auto Nemesis_Generation_Hash_String = "57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6";
#endif

		return test::ToArray<Hash256_Size>(Nemesis_Generation_Hash_String);
	}
}}
