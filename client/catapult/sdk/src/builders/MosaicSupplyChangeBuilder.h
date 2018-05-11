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
#include "plugins/txes/namespace/src/model/MosaicSupplyChangeTransaction.h"
#include "plugins/txes/namespace/src/model/NamespaceConstants.h"

namespace catapult { namespace builders {

	/// Builder for a mosaic supply change transaction.
	class MosaicSupplyChangeBuilder : public TransactionBuilder {
	public:
		using Transaction = model::MosaicSupplyChangeTransaction;
		using EmbeddedTransaction = model::EmbeddedMosaicSupplyChangeTransaction;

		/// Creates a mosaic supply change builder for building a mosaic supply change transaction for a mosaic (\a mosaicId)
		/// from \a signer for the network specified by \a networkIdentifier.
		MosaicSupplyChangeBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, MosaicId mosaicId);

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
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic supply change transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		MosaicId m_mosaicId;
		bool m_decrease;
		Amount m_delta;
	};
}}
