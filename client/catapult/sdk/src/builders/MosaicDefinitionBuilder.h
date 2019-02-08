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
#include "plugins/txes/mosaic/src/model/MosaicDefinitionTransaction.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a mosaic definition transaction.
	class MosaicDefinitionBuilder : public TransactionBuilder {
	public:
		using Transaction = model::MosaicDefinitionTransaction;
		using EmbeddedTransaction = model::EmbeddedMosaicDefinitionTransaction;

	public:
		/// Creates a mosaic definition builder for building a mosaic definition transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		MosaicDefinitionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the mosaic nonce to \a mosaicNonce.
		void setMosaicNonce(MosaicNonce mosaicNonce);

		/// Sets the mosaic flags to \a flags.
		void setFlags(model::MosaicFlags flags);

		/// Sets the mosaic divisibility to \a divisibility.
		void setDivisibility(uint8_t divisibility);

		/// Adds \a property to optional properties.
		void addProperty(const model::MosaicProperty& property);

	public:
		/// Builds a new mosaic definition transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic definition transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		MosaicNonce m_mosaicNonce;
		MosaicId m_mosaicId;
		model::MosaicFlags m_flags;
		uint8_t m_divisibility;
		std::vector<model::MosaicProperty> m_properties;
	};
}}
