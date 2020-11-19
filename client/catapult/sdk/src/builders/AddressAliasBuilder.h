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
#include "plugins/txes/namespace/src/model/AddressAliasTransaction.h"

namespace catapult { namespace builders {

	/// Builder for an address alias transaction.
	class AddressAliasBuilder : public TransactionBuilder {
	public:
		using Transaction = model::AddressAliasTransaction;
		using EmbeddedTransaction = model::EmbeddedAddressAliasTransaction;

	public:
		/// Creates an address alias builder for building an address alias transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		AddressAliasBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the identifier of the namespace that will become an alias to \a namespaceId.
		void setNamespaceId(NamespaceId namespaceId);

		/// Sets the aliased address to \a address.
		void setAddress(const Address& address);

		/// Sets the alias action to \a aliasAction.
		void setAliasAction(model::AliasAction aliasAction);

	public:
		/// Gets the size of address alias transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new address alias transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded address alias transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		NamespaceId m_namespaceId;
		Address m_address;
		model::AliasAction m_aliasAction;
	};
}}
