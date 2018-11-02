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
#include "MosaicEntityType.h"
#include "NamespaceConstants.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Parameters for a rule.
	struct RuleDefinition {
		/// Lower bound.
		uint64_t LowerBound;

		/// Upper bound.
		uint64_t UpperBound;

		/// Percentile.
		uint64_t Percentile;
	};

	/// Binary layout for a mosaic levy change transaction.
	struct MosaicLevyChangeTransaction : public Transaction {
	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 1;

		/// Transaction type.
		static constexpr EntityType Entity_Type = Entity_Type_Mosaic_Levy_Change;

	public:
		/// Id of the affected mosaic.
		catapult::MosaicId MosaicId;

		/// Id of the mosaic levy.
		catapult::MosaicId LevyId;

		/// Recipient of the levy.
		UnresolvedAddress Recipient;

		/// Rule ids for the transfers between accounts.
		std::array<uint8_t, Num_Mosaic_Levy_Rule_Ids> RuleIds;

		/// Number of rule definitions in the attachment.
		uint8_t RuleDefinitionCount;

		// followed by rule definitions

	private:
		template<typename T>
		static auto RuleDefinitionPtrT(T& transaction) {
			return transaction.RuleDefinitionCount ? PayloadStart(transaction) : nullptr;
		}

	public:
		/// Returns a const pointer to the rule definitions.
		const RuleDefinition* RuleDefinitionPtr() const {
			return reinterpret_cast<const RuleDefinition*>(RuleDefinitionPtrT(*this));
		}

		/// Returns a pointer to the rule definitions.
		RuleDefinition* RuleDefinitionPtr() {
			return reinterpret_cast<RuleDefinition*>(RuleDefinitionPtrT(*this));
		}

	public:
		/// Calculates the real size of a mosaic levy change \a transaction.
		static constexpr uint64_t CalculateRealSize(const MosaicLevyChangeTransaction& transaction) noexcept {
			return sizeof(MosaicLevyChangeTransaction) + transaction.RuleDefinitionCount * sizeof(RuleDefinition);
		}
	};

#pragma pack(pop)
}}
