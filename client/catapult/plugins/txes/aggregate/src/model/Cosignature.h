#pragma once
#include "catapult/types.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// A cosignature.
	struct Cosignature {
		/// The signer.
		Key Signer;

		/// The signature.
		catapult::Signature Signature;
	};

#pragma pack(pop)
}}
