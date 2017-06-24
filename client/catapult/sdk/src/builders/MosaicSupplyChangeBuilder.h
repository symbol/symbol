#pragma once
#include "TransactionBuilder.h"
#include "plugins/txes/namespace/src/model/MosaicSupplyChangeTransaction.h"
#include "plugins/txes/namespace/src/model/NamespaceConstants.h"

namespace catapult { namespace builders {

	/// Builder for a mosaic supply change transaction.
	class MosaicSupplyChangeBuilder : public TransactionBuilder<model::MosaicSupplyChangeTransaction> {
	public:
		/// Creates a mosaic supply change builder for building a mosaic supply change transaction for a mosaic (\a mosaicId)
		/// from \a signer for the network specified by \a networkIdentifier.
		MosaicSupplyChangeBuilder(
				model::NetworkIdentifier networkIdentifier,
				const Key& signer,
				MosaicId mosaicId);

		/// Creates a mosaic supply change builder for building a mosaic supply change transaction for a mosaic
		/// inside namespace (\a parentId) with \a name from \a signer for the network specified by \a networkIdentifier.
		MosaicSupplyChangeBuilder(
				model::NetworkIdentifier networkIdentifier,
				const Key& signer,
				NamespaceId parentId,
				const RawString& name);

	public:
		/// Sets the supply change direction to decrease.
		void setDecrease();

		/// Sets the supply \a delta.
		void setDelta(Amount delta);

	public:
		/// Builds a new mosaic supply change transaction.
		std::unique_ptr<model::MosaicSupplyChangeTransaction> build();

	private:
		MosaicId m_mosaicId;
		bool m_decrease;
		Amount m_delta;
	};
}}
