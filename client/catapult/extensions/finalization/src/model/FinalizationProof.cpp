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

#include "FinalizationProof.h"
#include <algorithm>

namespace catapult { namespace model {

	size_t GetMessageGroupPayloadSize(const FinalizationProofHeader& header) {
		return header.Size - sizeof(FinalizationProofHeader);
	}

	bool IsSizeValid(const FinalizationProof& proof) {
		if (proof.Size < sizeof(FinalizationProofHeader)) {
			CATAPULT_LOG(warning) << "proof failed size validation with size " << proof.Size;
			return false;
		}

		auto messageGroups = proof.MessageGroups(EntityContainerErrorPolicy::Suppress);
		auto areAllMessageGroupsValid = std::all_of(messageGroups.cbegin(), messageGroups.cend(), IsSizeValidT<FinalizationMessageGroup>);
		if (areAllMessageGroupsValid && !messageGroups.hasError())
			return true;

		CATAPULT_LOG(warning)
				<< "proof message groups failed size validation (valid sizes? " << areAllMessageGroupsValid
				<< ", errors? " << messageGroups.hasError() << ")";
		return false;
	}
}}
