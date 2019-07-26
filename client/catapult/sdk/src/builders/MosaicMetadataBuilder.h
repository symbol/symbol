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
#include "plugins/txes/metadata/src/model/MosaicMetadataTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a mosaic metadata transaction.
	class MosaicMetadataBuilder : public TransactionBuilder {
	public:
		using Transaction = model::MosaicMetadataTransaction;
		using EmbeddedTransaction = model::EmbeddedMosaicMetadataTransaction;

	public:
		/// Creates a mosaic metadata builder for building a mosaic metadata transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		MosaicMetadataBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the public key of the metadata target to \a targetPublicKey.
		void setTargetPublicKey(const Key& targetPublicKey);

		/// Sets the metadata key scoped to source, target and type to \a scopedMetadataKey.
		void setScopedMetadataKey(uint64_t scopedMetadataKey);

		/// Sets the metadata target identifier to \a targetId.
		void setTargetId(UnresolvedMosaicId targetId);

		/// Sets the change in value size in bytes to \a valueSizeDelta.
		void setValueSizeDelta(int16_t valueSizeDelta);

		/// Sets the value data to \a value.
		void setValue(const RawBuffer& value);

	public:
		/// Returns size of mosaic metadata transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new mosaic metadata transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic metadata transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		Key m_targetPublicKey;
		uint64_t m_scopedMetadataKey;
		UnresolvedMosaicId m_targetId;
		int16_t m_valueSizeDelta;
		std::vector<uint8_t> m_value;
	};
}}
