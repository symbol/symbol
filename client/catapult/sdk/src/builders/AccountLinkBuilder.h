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
#include "plugins/txes/account_link/src/model/AccountLinkTransaction.h"

namespace catapult { namespace builders {

	/// Builder for an account link transaction.
	class AccountLinkBuilder : public TransactionBuilder {
	public:
		using Transaction = model::AccountLinkTransaction;
		using EmbeddedTransaction = model::EmbeddedAccountLinkTransaction;

	public:
		/// Creates an account link builder for building an account link transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		AccountLinkBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the remote public key to \a remotePublicKey.
		void setRemotePublicKey(const Key& remotePublicKey);

		/// Sets the account link action to \a linkAction.
		void setLinkAction(model::AccountLinkAction linkAction);

	public:
		/// Returns size of account link transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new account link transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded account link transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		Key m_remotePublicKey;
		model::AccountLinkAction m_linkAction;
	};
}}
