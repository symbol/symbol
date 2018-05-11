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
#include "catapult/model/NetworkInfo.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a register namespace transaction.
	class RegisterNamespaceBuilder : public TransactionBuilder {
	public:
		using Transaction = model::RegisterNamespaceTransaction;
		using EmbeddedTransaction = model::EmbeddedRegisterNamespaceTransaction;

		/// Creates a register namespace builder for building a namespace registration transaction for \a name
		/// from \a signer for the network specified by \a networkIdentifier.
		RegisterNamespaceBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, const RawString& name);

	public:
		/// Sets the namespace \a duration.
		void setDuration(BlockDuration duration);

		/// Sets the namespace parent id (\a parentId).
		void setParentId(NamespaceId parentId);

	public:
		/// Builds a new register namespace transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded register namespace transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		// properties
		NamespaceId m_parentId;
		std::string m_name;
		BlockDuration m_duration;
	};
}}
