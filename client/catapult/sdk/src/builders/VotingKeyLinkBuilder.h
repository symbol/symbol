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
#include "plugins/txes/../coresystem/src/model/VotingKeyLinkTransaction.h"

namespace catapult { namespace builders {

	/// Builder for a voting key link transaction.
	class VotingKeyLinkBuilder : public TransactionBuilder {
	public:
		using Transaction = model::VotingKeyLinkTransaction;
		using EmbeddedTransaction = model::EmbeddedVotingKeyLinkTransaction;

	public:
		/// Creates a voting key link builder for building a voting key link transaction from \a signer
		/// for the network specified by \a networkIdentifier.
		VotingKeyLinkBuilder(model::NetworkIdentifier networkIdentifier, const Key& signer);

	public:
		/// Sets the linked public key to \a linkedPublicKey.
		void setLinkedPublicKey(const VotingKey& linkedPublicKey);

		/// Sets the start point to \a startPoint.
		void setStartPoint(FinalizationPoint startPoint);

		/// Sets the end point to \a endPoint.
		void setEndPoint(FinalizationPoint endPoint);

		/// Sets the link action to \a linkAction.
		void setLinkAction(model::LinkAction linkAction);

	public:
		/// Gets the size of voting key link transaction.
		/// \note This returns size of a normal transaction not embedded transaction.
		size_t size() const;

		/// Builds a new voting key link transaction.
		std::unique_ptr<Transaction> build() const;

		/// Builds a new embedded voting key link transaction.
		std::unique_ptr<EmbeddedTransaction> buildEmbedded() const;

	private:
		template<typename TTransaction>
		size_t sizeImpl() const;

		template<typename TTransaction>
		std::unique_ptr<TTransaction> buildImpl() const;

	private:
		VotingKey m_linkedPublicKey;
		FinalizationPoint m_startPoint;
		FinalizationPoint m_endPoint;
		model::LinkAction m_linkAction;
	};
}}
