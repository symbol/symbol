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

	// region ots options

	/// Ots tree options.
	struct OtsOptions {
		/// Max rounds.
		uint64_t MaxRounds;

		/// Max sub rounds.
		uint64_t MaxSubRounds;

	public:
		/// Returns \c true if these options are equal to \a rhs.
		bool operator==(const OtsOptions& rhs) const;

		/// Returns \c true if these options are not equal to \a rhs.
		bool operator!=(const OtsOptions& rhs) const;
	};

	// endregion

#pragma pack(push, 1)

	// region step identifier

	/// Finalization step identifier.
	struct StepIdentifier {
		/// Finalization point.
		uint64_t Point;

		/// Round.
		uint64_t Round;

		/// Sub round.
		uint64_t SubRound;

	public:
		/// Returns \c true if this step identifier is equal to \a rhs.
		bool operator==(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is not equal to \a rhs.
		bool operator!=(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is less than \a rhs.
		bool operator<(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is less than or equal to \a rhs.
		bool operator<=(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is greater than \a rhs.
		bool operator>(const StepIdentifier& rhs) const;

		/// Returns \c true if this step identifier is greater than or equal to \a rhs.
		bool operator>=(const StepIdentifier& rhs) const;
	};

	/// Insertion operator for outputting \a stepIdentifier to \a out.
	std::ostream& operator<<(std::ostream& out, const StepIdentifier& stepIdentifier);

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

		/// Middle pair.
		OtsParentPublicKeySignaturePair Middle;

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
