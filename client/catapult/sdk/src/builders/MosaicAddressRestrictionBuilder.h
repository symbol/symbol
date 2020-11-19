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
#include "plugins/txes/restriction_mosaic/src/model/MosaicAddressRestrictionTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a mosaic address restriction transaction.
	class MosaicAddressRestrictionBuilder : public TransactionBuilder {
	public:
		using Transaction = model::MosaicAddressRestrictionTransaction;
		using EmbeddedTransaction = model::EmbeddedMosaicAddressRestrictionTransaction;

	public:
		/// Creates a mosaic address restriction builder for building a mosaic address restriction transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		MosaicAddressRestrictionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the identifier of the mosaic to which the restriction applies to \a mosaicId.
		void setMosaicId(UnresolvedMosaicId mosaicId);

		/// Sets the restriction key to \a restrictionKey.
		void setRestrictionKey(uint64_t restrictionKey);

		/// Sets the previous restriction value to \a previousRestrictionValue.
		void setPreviousRestrictionValue(uint64_t previousRestrictionValue);

		/// Sets the new restriction value to \a newRestrictionValue.
		void setNewRestrictionValue(uint64_t newRestrictionValue);

		/// Sets the address being restricted to \a targetAddress.
		void setTargetAddress(const UnresolvedAddress& targetAddress);

	public:
		/// Gets the size of mosaic address restriction transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new mosaic address restriction transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic address restriction transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		UnresolvedMosaicId m_mosaicId;
		uint64_t m_restrictionKey;
		uint64_t m_previousRestrictionValue;
		uint64_t m_newRestrictionValue;
		UnresolvedAddress m_targetAddress;
	};
}}
