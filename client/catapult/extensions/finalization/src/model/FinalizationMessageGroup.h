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
#include "catapult/crypto_voting/OtsTypes.h"
#include "catapult/model/SizePrefixedEntity.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Finalization message group.
	struct FinalizationMessageGroup : public SizePrefixedEntity {
	private:
		template<typename T>
		static auto* HashesPtrT(T& messageGroup) {
			return messageGroup.HashesCount ? PayloadStart(messageGroup) : nullptr;
		}

		template<typename T>
		static auto* SignaturesPtrT(T& messageGroup) {
			auto* pPayloadStart = PayloadStart(messageGroup);
			return messageGroup.SignaturesCount && pPayloadStart
					? pPayloadStart + messageGroup.HashesCount * Hash256::Size
					: nullptr;
		}

	public:
		/// Number of hashes.
		uint16_t HashesCount;

		/// Number of signatures.
		uint16_t SignaturesCount;

		/// Message stage.
		FinalizationStage Stage;

		/// Block height corresponding to the the first hash.
		catapult::Height Height;

		// followed by hashes data if HashesCount != 0
		DEFINE_SIZE_PREFIXED_ENTITY_VARIABLE_DATA_ACCESSORS(Hashes, Hash256)

		// followed by signature data if SignaturesCount != 0
		DEFINE_SIZE_PREFIXED_ENTITY_VARIABLE_DATA_ACCESSORS(Signatures, crypto::OtsTreeSignature)

	public:
		/// Calculates the real size of finalization message group (\a messageGroup).
		static constexpr uint64_t CalculateRealSize(const FinalizationMessageGroup& messageGroup) noexcept {
			return sizeof(FinalizationMessageGroup)
					+ messageGroup.HashesCount * Hash256::Size
					+ messageGroup.SignaturesCount * sizeof(crypto::OtsTreeSignature);
		}
	};

#pragma pack(pop)
}}
