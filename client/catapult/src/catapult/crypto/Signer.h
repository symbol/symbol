#pragma once
#include "KeyPair.h"
#include <vector>

namespace catapult { namespace crypto {

	/// Signs data pointed by \a dataBuffer using \a keyPair, placing resulting signature
	/// in \a computedSignature.
	/// \note The function will throw if the generated S part of the signature is not less than the group order.
	void Sign(const KeyPair& keyPair, const RawBuffer& dataBuffer, Signature& computedSignature);

	/// Signs data in \a buffersList using \a keyPair, placing resulting signature
	/// in \a computedSignature.
	/// \note The function will throw if the generated S part of the signature is not less than the group order.
	void Sign(const KeyPair& keyPair, std::initializer_list<const RawBuffer> buffersList, Signature& computedSignature);

	/// Verifies that \a signature of data pointed by \a dataBuffer is valid, using public key
	/// \a publicKey.
	/// Returns \c true if signature is valid.
	bool Verify(const Key& publicKey, const RawBuffer& dataBuffer, const Signature& signature);

	/// Verifies that \a signature of data in \a buffersList is valid, using public key
	/// \a publicKey.
	/// Returns \c true if signature is valid.
	bool Verify(const Key& publicKey, std::initializer_list<const RawBuffer> buffersList, const Signature& signature);
}}
