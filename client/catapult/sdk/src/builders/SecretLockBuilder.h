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
#include "plugins/txes/lock/src/model/LockTypes.h"
#include "plugins/txes/lock/src/model/SecretLockTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a secret lock transaction.
	class SecretLockBuilder : public TransactionBuilder {
	public:
		using Transaction = model::SecretLockTransaction;
		using EmbeddedTransaction = model::EmbeddedSecretLockTransaction;

		/// Creates a secret lock builder for building a secret lock transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		SecretLockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the mosaic with \a mosaicId and \a amount.
		void setMosaic(MosaicId mosaicId, Amount amount);

		/// Sets the \a duration.
		void setDuration(BlockDuration duration);

		// Sets the \a hashAlgorithm.
		void setHashAlgorithm(model::LockHashAlgorithm hashAlgorithm);

		// Sets the \a secret.
		void setSecret(const Hash512& secret);

		// Sets the \a recipient.
		void setRecipient(const Address& recipient);

	public:
		/// Builds a new secret lock transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded secret lock transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		model::Mosaic m_mosaic;
		BlockDuration m_duration;
		model::LockHashAlgorithm m_hashAlgorithm;
		Hash512 m_secret;
		Address m_recipient;
	};
}}
