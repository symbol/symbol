#pragma once
#include "MosaicTypes.h"
#include "catapult/model/EntityType.h"
#include "catapult/model/Transaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Binary layout for a mosaic supply change transaction body.
	template<typename THeader>
	struct MosaicSupplyChangeTransactionBody : public THeader {
	private:
		using TransactionType = MosaicSupplyChangeTransactionBody<THeader>;

	public:
		/// Transaction format version.
		static constexpr uint8_t Current_Version = 2;

		/// Transaction type.
		static constexpr EntityType Entity_Type = EntityType::Mosaic_Supply_Change;

	public:
		/// The id of the affected mosaic.
		catapult::MosaicId MosaicId;

		/// The supply change direction.
		MosaicSupplyChangeDirection Direction;

		/// The amount of the change.
		Amount Delta;

	public:
		/// Calculates the real size of a mosaic supply change \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType&) noexcept {
			return sizeof(TransactionType);
		}
	};

	DEFINE_EMBEDDABLE_TRANSACTION(MosaicSupplyChange)

#pragma pack(pop)
}}
