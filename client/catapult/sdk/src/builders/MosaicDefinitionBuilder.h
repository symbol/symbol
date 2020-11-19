/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
		/// Sets the mosaic duration to \a duration.
		void setDuration(BlockDuration duration);

		/// Sets the mosaic nonce to \a nonce.
		void setNonce(MosaicNonce nonce);

		/// Sets the mosaic flags to \a flags.
		void setFlags(model::MosaicFlags flags);

		/// Sets the mosaic divisibility to \a divisibility.
		void setDivisibility(uint8_t divisibility);

	public:
		/// Gets the size of mosaic definition transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new mosaic definition transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic definition transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		MosaicId m_id;
		BlockDuration m_duration;
		MosaicNonce m_nonce;
		model::MosaicFlags m_flags;
		uint8_t m_divisibility;
	};
}}
