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

	/// A detached cosignature.
	struct DetachedCosignature : public Cosignature {
	public:
		/// Creates a detached cosignature around \a signer, \a signature and \a parentHash.
		DetachedCosignature(const Key& signer, const catapult::Signature& signature, const Hash256& parentHash)
				: Cosignature{ signer, signature }
				, ParentHash(parentHash)
		{}

	public:
		/// The hash of the corresponding parent.
		Hash256 ParentHash;
	};

#pragma pack(pop)
}}
