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

#include "SecretLockBuilder.h"

namespace catapult { namespace builders {

	SecretLockBuilder::SecretLockBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer)
			: TransactionBuilder(networkIdentifier, signer)
			, m_hashAlgorithm(model::LockHashAlgorithm::Op_Sha3)
			, m_secret()
			, m_recipient()
	{}

	void SecretLockBuilder::setMosaic(UnresolvedMosaicId mosaicId, Amount amount) {
		m_mosaic = { mosaicId, amount };
	}

	void SecretLockBuilder::setDuration(BlockDuration duration) {
		m_duration = duration;
	}

	void SecretLockBuilder::setHashAlgorithm(model::LockHashAlgorithm hashAlgorithm) {
		m_hashAlgorithm = hashAlgorithm;
	}

	void SecretLockBuilder::setSecret(const Hash512& secret) {
		m_secret = secret;
	}

	void SecretLockBuilder::setRecipient(const UnresolvedAddress& recipient) {
		m_recipient = recipient;
	}

	template<typename TransactionType>
	std::unique_ptr<TransactionType> SecretLockBuilder::buildImpl() const {
		// 1. allocate, zero (header), set model::Transaction fields
		auto pTransaction = createTransaction<TransactionType>(sizeof(TransactionType));

		// 2. set transaction fields
		pTransaction->Mosaic = m_mosaic;
		pTransaction->Duration = m_duration;
		pTransaction->HashAlgorithm = m_hashAlgorithm;
		pTransaction->Secret = m_secret;
		pTransaction->Recipient = m_recipient;

		return pTransaction;
	}

	std::unique_ptr<SecretLockBuilder::Transaction> SecretLockBuilder::build() const {
		return buildImpl<Transaction>();
	}

	std::unique_ptr<SecretLockBuilder::EmbeddedTransaction> SecretLockBuilder::buildEmbedded() const {
		return buildImpl<EmbeddedTransaction>();
	}
}}
