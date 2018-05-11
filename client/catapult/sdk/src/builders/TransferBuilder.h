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
#include <map>
#include <vector>

namespace catapult { namespace builders {

	/// Builder for a transfer transaction.
	class TransferBuilder : public TransactionBuilder {
	public:
		using Transaction = model::TransferTransaction;
		using EmbeddedTransaction = model::EmbeddedTransferTransaction;

		/// Creates a transfer builder for building a transfer transaction from \a signer to \a recipient for the network specified by
		/// \a networkIdentifier.
		TransferBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer, const Address& recipient);

	public:
		/// Sets the transfer message to \a message.
		void setMessage(const RawBuffer& message);

		/// Sets the transfer message to \a message.
		void setStringMessage(const RawString& message);

	public:
		/// Adds a transfer of \a mosaicId with \a amount.
		void addMosaic(MosaicId mosaicId, Amount amount);

		/// Adds a transfer of \a mosaicName with \a amount.
		void addMosaic(const RawString& mosaicName, Amount amount);

	public:
		/// Builds a new transfer transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded transfer transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		const Address& m_recipient;
		std::vector<uint8_t> m_message;
		std::map<MosaicId, Amount> m_mosaicTransfers;
	};
}}
