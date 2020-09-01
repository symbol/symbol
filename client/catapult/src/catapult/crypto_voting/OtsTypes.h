/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

namespace catapult { namespace crypto {

	using OtsPublicKey = Key;
	using OtsSignature = Signature;

#pragma pack(push, 1)

	// region ots key identifier

	/// Ots key identifier.
	struct OtsKeyIdentifier {
		/// Batch id.
		uint64_t BatchId;

		/// Key id.
		uint64_t KeyId;

	public:
		/// Returns \c true if this ots key identifier is equal to \a rhs.
		bool operator==(const OtsKeyIdentifier& rhs) const;

		/// Returns \c true if this ots key identifier is not equal to \a rhs.
		bool operator!=(const OtsKeyIdentifier& rhs) const;

		/// Returns \c true if this ots key identifier is less than \a rhs.
		bool operator<(const OtsKeyIdentifier& rhs) const;

		/// Returns \c true if this ots key identifier is less than or equal to \a rhs.
		bool operator<=(const OtsKeyIdentifier& rhs) const;

		/// Returns \c true if this ots key identifier is greater than \a rhs.
		bool operator>(const OtsKeyIdentifier& rhs) const;

		/// Returns \c true if this ots key identifier is greater than or equal to \a rhs.
		bool operator>=(const OtsKeyIdentifier& rhs) const;
	};

	/// Insertion operator for outputting \a keyIdentifier to \a out.
	std::ostream& operator<<(std::ostream& out, const OtsKeyIdentifier& keyIdentifier);

	// endregion

	// region ots options

	/// Ots tree options.
	struct OtsOptions {
		/// Key dilution.
		uint64_t Dilution;

		/// Start key identifier.
		OtsKeyIdentifier StartKeyIdentifier;

		/// End key identifier.
		OtsKeyIdentifier EndKeyIdentifier;
	};

	// endregion

	// region public key signature pair

	/// Signature pair.
	struct OtsParentPublicKeySignaturePair {
		/// Public key.
		OtsPublicKey ParentPublicKey;

		/// Signature.
		OtsSignature Signature;
	};

	// endregion

	// region ots tree signature

	/// One-time signature.
	struct OtsTreeSignature {
		/// Root pair.
		OtsParentPublicKeySignaturePair Root;

		/// Top pair.
		OtsParentPublicKeySignaturePair Top;

		/// Bottom pair.
		OtsParentPublicKeySignaturePair Bottom;

	public:
		/// Returns \c true if this signature is equal to \a rhs.
		bool operator==(const OtsTreeSignature& rhs) const;

		/// Returns \c true if this signature is not equal to \a rhs.
		bool operator!=(const OtsTreeSignature& rhs) const;
	};

	// endregion

#pragma pack(pop)
}}
