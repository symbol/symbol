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
#include "StepIdentifier.h"
#include "catapult/model/TrailingVariableDataLayout.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Vote proof.
	struct VoteProof {
	public:
		/// Vote message signature.
		crypto::OtsTreeSignature Signature;
	};

	/// Packed finalization proof.
	struct PackedFinalizationProof : public model::TrailingVariableDataLayout<PackedFinalizationProof, VoteProof> {
		/// Number of vote proofs.
		uint32_t VoteProofsCount;

		/// Hash of finalized block or special 'empty' hash.
		Hash256 FinalizedHash;

		/// Height of finalized block.
		Height FinalizedHeight;

		/// Finalization step.
		model::StepIdentifier StepIdentifier;

	public:
		/// Gets a const pointer to the first vote contained in this proof.
		const VoteProof* VoteProofsPtr() const {
			return VoteProofsCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

		/// Gets a pointer to the first vote contained in this proof.
		VoteProof* VoteProofsPtr() {
			return VoteProofsCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
		}

	public:
		/// Calculates the real size of \a proof.
		static constexpr uint64_t CalculateRealSize(const PackedFinalizationProof& proof) noexcept {
			return sizeof(PackedFinalizationProof) + proof.VoteProofsCount * sizeof(VoteProof);
		}
	};

#pragma pack(pop)
}}
