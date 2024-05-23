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
#include "plugins/txes/metadata/src/model/NamespaceMetadataTransaction.h"

namespace catapult {
namespace builders {

	/// Builder for a namespace metadata transaction.
	class NamespaceMetadataBuilder : public TransactionBuilder {
	public:
		using Transaction = model::NamespaceMetadataTransaction;
		using EmbeddedTransaction = model::EmbeddedNamespaceMetadataTransaction;

	public:
		/// Creates a namespace metadata builder for building a namespace metadata transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		NamespaceMetadataBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the metadata target address to \a targetAddress.
		void setTargetAddress(const UnresolvedAddress& targetAddress);

		/// Sets the metadata key scoped to source, target and type to \a scopedMetadataKey.
		void setScopedMetadataKey(uint64_t scopedMetadataKey);

		/// Sets the target namespace identifier to \a targetNamespaceId.
		void setTargetNamespaceId(NamespaceId targetNamespaceId);

		/// Sets the change in value size in bytes to \a valueSizeDelta.
		void setValueSizeDelta(int16_t valueSizeDelta);

		/// Sets the difference between existing value and new value to \a value.
		/// \note When there is no existing value, new value is same this value.
		/// \note When there is an existing value, new value is calculated as xor(previous-value, value).
		void setValue(const RawBuffer& value);

	public:
		/// Gets the size of namespace metadata transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new namespace metadata transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded namespace metadata transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template <typename TTransaction>
		size_t sizeImpl() const;

		template <typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		UnresolvedAddress m_targetAddress;
		uint64_t m_scopedMetadataKey;
		NamespaceId m_targetNamespaceId;
		int16_t m_valueSizeDelta;
		std::vector<uint8_t> m_value;
	};
}
}
