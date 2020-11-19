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
#include "plugins/txes/restriction_mosaic/src/model/MosaicGlobalRestrictionTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a mosaic global restriction transaction.
	class MosaicGlobalRestrictionBuilder : public TransactionBuilder {
	public:
		using Transaction = model::MosaicGlobalRestrictionTransaction;
		using EmbeddedTransaction = model::EmbeddedMosaicGlobalRestrictionTransaction;

	public:
		/// Creates a mosaic global restriction builder for building a mosaic global restriction transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		MosaicGlobalRestrictionBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the identifier of the mosaic being restricted to \a mosaicId.
		void setMosaicId(UnresolvedMosaicId mosaicId);

		/// Sets the identifier of the mosaic providing the restriction key to \a referenceMosaicId.
		void setReferenceMosaicId(UnresolvedMosaicId referenceMosaicId);

		/// Sets the restriction key relative to the reference mosaic identifier to \a restrictionKey.
		void setRestrictionKey(uint64_t restrictionKey);

		/// Sets the previous restriction value to \a previousRestrictionValue.
		void setPreviousRestrictionValue(uint64_t previousRestrictionValue);

		/// Sets the new restriction value to \a newRestrictionValue.
		void setNewRestrictionValue(uint64_t newRestrictionValue);

		/// Sets the previous restriction type to \a previousRestrictionType.
		void setPreviousRestrictionType(model::MosaicRestrictionType previousRestrictionType);

		/// Sets the new restriction type to \a newRestrictionType.
		void setNewRestrictionType(model::MosaicRestrictionType newRestrictionType);

	public:
		/// Gets the size of mosaic global restriction transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new mosaic global restriction transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic global restriction transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		UnresolvedMosaicId m_mosaicId;
		UnresolvedMosaicId m_referenceMosaicId;
		uint64_t m_restrictionKey;
		uint64_t m_previousRestrictionValue;
		uint64_t m_newRestrictionValue;
		model::MosaicRestrictionType m_previousRestrictionType;
		model::MosaicRestrictionType m_newRestrictionType;
	};
}}
