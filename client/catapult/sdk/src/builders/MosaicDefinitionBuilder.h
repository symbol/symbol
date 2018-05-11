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
#include "plugins/txes/namespace/src/model/MosaicDefinitionTransaction.h"
#include "plugins/txes/namespace/src/model/MosaicProperty.h"
#include "catapult/model/NetworkInfo.h"
#include <map>
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a mosaic definition transaction.
	class MosaicDefinitionBuilder : public TransactionBuilder {
	public:
		using Transaction = model::MosaicDefinitionTransaction;
		using EmbeddedTransaction = model::EmbeddedMosaicDefinitionTransaction;

		/// Creates a mosaic definition builder for building a mosaic definition transaction for a mosaic inside namespace (\a parentId)
		/// with \a name from \a signer for the network specified by \a networkIdentifier.
		MosaicDefinitionBuilder(
				model::NetworkIdentifier networkIdentifier,
				const Key& signer,
				NamespaceId parentId,
				const RawString& name);

	public:
		/// Sets the mosaic supply mutability.
		void setSupplyMutable();

		/// Sets the mosaic transferability.
		void setTransferable();

		/// Sets the mosaic levy mutability.
		void setLevyMutable();

		/// Sets the mosaic \a divisibility.
		void setDivisibility(uint8_t divisibility);

		/// Sets the mosaic \a duration.
		void setDuration(BlockDuration duration);

	public:
		/// Builds a new mosaic definition transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded mosaic definition transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	protected:
		void addOptionalProperty(model::MosaicPropertyId propertyId, uint64_t value) {
			m_optionalProperties[propertyId] = value;
		}

		void dropOptionalProperty(model::MosaicPropertyId propertyId) {
			m_optionalProperties.erase(propertyId);
		}

	private:
		// properties
		NamespaceId m_parentId;
		std::string m_name;

		model::MosaicFlags m_flags;
		uint8_t m_divisibility;
		std::map<model::MosaicPropertyId, uint64_t> m_optionalProperties;
	};
}}
