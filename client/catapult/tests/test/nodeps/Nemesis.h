#pragma once
#include "Conversions.h"

namespace catapult { namespace test {

	/// Gets the nemesis generation hash.
	CATAPULT_INLINE
	Hash256 GetNemesisGenerationHash() {
#ifdef SIGNATURE_SCHEME_NIS1
		constexpr auto Nemesis_Generation_Hash_String = "16ED3D69d3CA67132AACE4405AA122E5E041E58741A4364255B15201F5AAF6E4";
#else
		constexpr auto Nemesis_Generation_Hash_String = "57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6";
#endif

		return test::ToArray<Hash256_Size>(Nemesis_Generation_Hash_String);
	}
}}
