#pragma once
#include "MosaicEntityType.h"
#include "NamespaceConstants.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Parameters for a rule.
	struct RuleDefinition {
		/// The lower bound.
		uint64_t LowerBound;

		/// The upper bound.
		uint64_t UpperBound;

		/// The percentile.
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
		/// The id of the affected mosaic.
		catapult::MosaicId MosaicId;

		/// The id of the mosaic levy.
		catapult::MosaicId LevyId;

		/// The recipient of the levy.
		Address Recipient;

		/// The rule ids for the transfers between accounts.
		std::array<uint8_t, Num_Mosaic_Levy_Rule_Ids> RuleIds;

		/// The number of rule definitions in the attachment.
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
