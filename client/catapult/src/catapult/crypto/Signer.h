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
#include "KeyPair.h"
#include <vector>

namespace catapult { namespace crypto {

	/// Signature input.
	struct SignatureInput {
		/// Public key.
		const Key& PublicKey;

		/// Buffers.
		std::vector<RawBuffer> Buffers;

		/// Signature.
		const catapult::Signature& Signature;
	};

	/// Signs data pointed by \a dataBuffer using \a keyPair, placing resulting signature in \a computedSignature.
	/// \note The function will throw if the generated S part of the signature is not less than the group order.
	void Sign(const KeyPair& keyPair, const RawBuffer& dataBuffer, Signature& computedSignature);

	/// Signs data in \a buffersList using \a keyPair, placing resulting signature in \a computedSignature.
	/// \note The function will throw if the generated S part of the signature is not less than the group order.
	void Sign(const KeyPair& keyPair, std::initializer_list<const RawBuffer> buffersList, Signature& computedSignature);

	/// Verifies that \a signature of data pointed by \a dataBuffer is valid, using public key \a publicKey.
	/// Returns \c true if signature is valid.
	bool Verify(const Key& publicKey, const RawBuffer& dataBuffer, const Signature& signature);

	/// Verifies that \a signature of data in \a buffersList is valid, using public key \a publicKey.
	/// Returns \c true if signature is valid.
	bool Verify(const Key& publicKey, const std::vector<RawBuffer>& buffersList, const Signature& signature);

	/// Generates a specified number of random bytes into an output buffer.
	using RandomFiller = consumer<uint8_t*, size_t>;

	/// Verifies that all \a count signatures pointed to by \a pSignatureInputs are valid.
	/// \a randomFiller is used to generate random bytes.
	/// Collates and returns a pair consisting of an aggregate result that is \c true when all signatures are valid
	/// and a vector of bools that indicates the verification result for each individual signature.
	std::pair<std::vector<bool>, bool> VerifyMulti(const RandomFiller& randomFiller, const SignatureInput* pSignatureInputs, size_t count);

	/// Verifies that all \a count signatures pointed to by \a pSignatureInputs are valid.
	/// \a randomFiller is used to generate random bytes.
	/// Collates and returns an aggregate result that is \c true when all signatures are valid.
	bool VerifyMultiShortCircuit(const RandomFiller& randomFiller, const SignatureInput* pSignatureInputs, size_t count);
}}
