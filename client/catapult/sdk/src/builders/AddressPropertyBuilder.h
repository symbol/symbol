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
#include "plugins/txes/property/src/model/AddressPropertyTransaction.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for an address property transaction.
	class AddressPropertyBuilder : public TransactionBuilder {
	public:
		using Transaction = model::AddressPropertyTransaction;
		using EmbeddedTransaction = model::EmbeddedAddressPropertyTransaction;

	public:
		/// Creates an address property builder for building an address property transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		AddressPropertyBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the property type to \a propertyType.
		void setPropertyType(model::PropertyType propertyType);

		/// Adds \a modification to property modifications.
		void addModification(const model::AddressPropertyModification& modification);

	public:
		/// Builds a new address property transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded address property transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		model::PropertyType m_propertyType;
		std::vector<model::AddressPropertyModification> m_modifications;
	};
}}
