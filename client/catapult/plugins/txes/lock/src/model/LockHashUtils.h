#pragma once
#include "LockTypes.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// Calculates \a hashAlgorithm hash of \a data.
	Hash512 CalculateHash(model::LockHashAlgorithm hashAlgorithm, const RawBuffer& data);
}}
