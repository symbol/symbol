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
#include "plugins/txes/namespace/src/model/RegisterNamespaceTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a register namespace transaction.
	class RegisterNamespaceBuilder : public TransactionBuilder {
	public:
		using Transaction = model::RegisterNamespaceTransaction;
		using EmbeddedTransaction = model::EmbeddedRegisterNamespaceTransaction;

	public:
		/// Creates a register namespace builder for building a register namespace transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		RegisterNamespaceBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the namespace duration to \a duration and namespaceType to `root`.
		void setDuration(BlockDuration duration);

		/// Sets the id of the parent namespace to \a parentId and namespaceType to `child`.
		void setParentId(NamespaceId parentId);

		/// Sets the namespace name to \a name.
		void setName(const RawBuffer& name);

	public:
		/// Builds a new register namespace transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded register namespace transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		model::NamespaceType m_namespaceType;
		BlockDuration m_duration;
		NamespaceId m_parentId;
		NamespaceId m_namespaceId;
		std::vector<uint8_t> m_name;
	};
}}
