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
#include "finalization/src/model/FinalizationMessage.h"
#include "finalization/src/model/FinalizationProof.h"
#include "finalization/src/model/FinalizationStatistics.h"
#include "catapult/model/HeightHashPair.h"
#include <memory>

namespace catapult { namespace io {

	/// Interface for saving and loading finalization proofs.
	class ProofStorage {
	public:
		virtual ~ProofStorage() = default;

	public:
		/// Gets the statistics of the last finalized block.
		virtual model::FinalizationStatistics statistics() const = 0;

		/// Gets the finalization proof at \a point.
		virtual std::shared_ptr<const model::FinalizationProof> loadProof(FinalizationPoint point) const = 0;

		/// Gets the first finalization proof at \a height.
		virtual std::shared_ptr<const model::FinalizationProof> loadProof(Height height) const = 0;

		/// Saves finalization \a proof.
		virtual void saveProof(const model::FinalizationProof& proof) = 0;
	};
}}
