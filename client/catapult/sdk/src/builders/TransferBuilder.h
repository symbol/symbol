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
#include "plugins/txes/transfer/src/model/TransferTransaction.h"
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a transfer transaction.
	class TransferBuilder : public TransactionBuilder {
	public:
		using Transaction = model::TransferTransaction;
		using EmbeddedTransaction = model::EmbeddedTransferTransaction;

	public:
		/// Creates a transfer builder for building a transfer transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		TransferBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the transaction recipient to \a recipient.
		void setRecipient(const UnresolvedAddress& recipient);

		/// Sets the transaction message to \a message.
		void setMessage(const RawBuffer& message);

		/// Adds \a mosaic to attached mosaics.
		void addMosaic(const model::UnresolvedMosaic& mosaic);

	public:
		/// Builds a new transfer transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded transfer transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		UnresolvedAddress m_recipient;
		std::vector<uint8_t> m_message;
		std::vector<model::UnresolvedMosaic> m_mosaics;
	};
}}
