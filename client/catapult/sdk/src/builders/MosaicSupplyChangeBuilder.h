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
#include "TransactionBuilder.h"
#include "plugins/txes/mosaic/src/model/MosaicSupplyChangeTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a mosaic supply change transaction.
	class MosaicSupplyChangeBuilder : public TransactionBuilder {
	public:
		using Transaction = model::MosaicSupplyChangeTransaction;
		using EmbeddedTransaction = model::EmbeddedMosaicSupplyChangeTransaction;

	public:
		/// Creates a mosaic supply change builder for building a mosaic supply change transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		MosaicSupplyChangeBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the id of the affected mosaic to \a mosaicId.
		void setMosaicId(UnresolvedMosaicId mosaicId);

		/// Sets the supply change direction to \a direction.
		void setDirection(model::MosaicSupplyChangeDirection direction);

		/// Sets the amount of the change to \a delta.
		void setDelta(Amount delta);

	public:
		/// Builds a new mosaic supply change transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic supply change transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		UnresolvedMosaicId m_mosaicId;
		model::MosaicSupplyChangeDirection m_direction;
		Amount m_delta;
	};
}}
