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
#include "plugins/txes/property/src/model/PropertyTransaction.h"
#include "plugins/txes/property/src/model/PropertyTypes.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a property transaction.
	template<typename TPropertyTransaction, typename TEmbeddedPropertyTransaction, typename TPropertyValue>
	class PropertyBuilder : public TransactionBuilder {
	public:
		using Transaction = TPropertyTransaction;
		using EmbeddedTransaction = TEmbeddedPropertyTransaction;
		using Modification = model::PropertyModification<TPropertyValue>;

		/// Creates a property builder for building a property transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		PropertyBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
				: TransactionBuilder(networkIdentifier, signer)
				, m_propertyType(model::PropertyType(0))
		{}

	public:
		/// Sets the property type (\a propertyType).
		void setPropertyType(model::PropertyType propertyType) {
			m_propertyType = propertyType;
		}

		/// Adds a property modification around \a type and \a value.
		void addPropertyModification(model::PropertyModificationType type, const TPropertyValue& value) {
			m_modifications.push_back(Modification{ type, value });
		}

	public:
		/// Builds a new property transaction.
		std::unique_ptr<TPropertyTransaction> build() const {
			return buildImpl<TPropertyTransaction>();
		}

		/// Builds a new embedded property transaction.
		std::unique_ptr<TEmbeddedPropertyTransaction> buildEmbedded() const {
			return buildImpl<TEmbeddedPropertyTransaction>();
		}

	private:
		template<typename TransactionType>
		std::unique_ptr<TransactionType> buildImpl() const {
			// 1. allocate, zero (header), set model::Transaction fields
			auto size = sizeof(TransactionType) + m_modifications.size() * sizeof(Modification);
			auto pTransaction = createTransaction<TransactionType>(size);

			// 2. set transaction fields
			pTransaction->PropertyType = m_propertyType;

			// 3. set sizes upfront, so that pointers are calculated correctly
			pTransaction->ModificationsCount = utils::checked_cast<size_t, uint8_t>(m_modifications.size());

			// 4. set modifications
			if (!m_modifications.empty())
				std::memcpy(pTransaction->ModificationsPtr(), m_modifications.data(), m_modifications.size() * sizeof(Modification));

			return pTransaction;
		}

	private:
		model::PropertyType m_propertyType;
		std::vector<Modification> m_modifications;
	};

	/// Address property builder.
	using AddressPropertyBuilder = PropertyBuilder<
		model::AddressPropertyTransaction,
		model::EmbeddedAddressPropertyTransaction,
		UnresolvedAddress>;

	/// Mosaic property builder.
	using MosaicPropertyBuilder = PropertyBuilder<
		model::MosaicPropertyTransaction,
		model::EmbeddedMosaicPropertyTransaction,
		UnresolvedMosaicId>;

	/// Transaction type property builder.
	using TransactionTypePropertyBuilder = PropertyBuilder<
		model::TransactionTypePropertyTransaction,
		model::EmbeddedTransactionTypePropertyTransaction,
		model::EntityType>;
}}
