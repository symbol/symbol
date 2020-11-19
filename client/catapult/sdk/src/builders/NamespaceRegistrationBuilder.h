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
#include "plugins/txes/namespace/src/model/NamespaceRegistrationTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a namespace registration transaction.
	class NamespaceRegistrationBuilder : public TransactionBuilder {
	public:
		using Transaction = model::NamespaceRegistrationTransaction;
		using EmbeddedTransaction = model::EmbeddedNamespaceRegistrationTransaction;

	public:
		/// Creates a namespace registration builder for building a namespace registration transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		NamespaceRegistrationBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the namespace duration to \a duration and registrationType to `root`.
		void setDuration(BlockDuration duration);

		/// Sets the parent namespace identifier to \a parentId and registrationType to `child`.
		void setParentId(NamespaceId parentId);

		/// Sets the namespace name to \a name.
		void setName(const RawBuffer& name);

	public:
		/// Gets the size of namespace registration transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new namespace registration transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded namespace registration transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		BlockDuration m_duration;
		NamespaceId m_parentId;
		NamespaceId m_id;
		model::NamespaceRegistrationType m_registrationType;
		std::vector<uint8_t> m_name;
	};
}}
