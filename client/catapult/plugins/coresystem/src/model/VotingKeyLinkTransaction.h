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
#include "catapult/model/KeyLinkSharedTransaction.h"

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Voting key format v1.
	template<typename THeader>
	struct VotingKeyV1 : public THeader {
	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Voting_Key_Link, 1)

	public:
		/// Linked public key.
		VotingKey LinkedPublicKey;

		std::array<uint8_t, 16> VotingKeyV1_Reserved1;
	};

	/// Voting key format v2.
	template<typename THeader>
	struct VotingKeyV2 : public THeader {
	public:
		DEFINE_TRANSACTION_CONSTANTS(Entity_Type_Voting_Key_Link, 2)

	public:
		/// Linked public key.
		VotingKey LinkedPublicKey;
	};

	/// Binary layout for a voting key link transaction body.
	template<typename THeader>
	struct BasicVotingKeyLinkTransactionBody : public THeader {
	private:
		using TransactionType = BasicVotingKeyLinkTransactionBody<THeader>;

	public:
		/// Start epoch.
		FinalizationEpoch StartEpoch;

		/// End epoch.
		FinalizationEpoch EndEpoch;

		/// Link action.
		model::LinkAction LinkAction;

	public:
		/// Calculates the real size of key link \a transaction.
		static constexpr uint64_t CalculateRealSize(const TransactionType&) noexcept {
			return sizeof(TransactionType);
		}
	};

	template<typename THeader>
	struct VotingKeyLinkV1TransactionBody
			: public BasicVotingKeyLinkTransactionBody<VotingKeyV1<THeader>>
	{};

	template<typename THeader>
	struct VotingKeyLinkTransactionBody
			: public BasicVotingKeyLinkTransactionBody<VotingKeyV2<THeader>>
	{};

	DEFINE_EMBEDDABLE_TRANSACTION(VotingKeyLinkV1)
	DEFINE_EMBEDDABLE_TRANSACTION(VotingKeyLink)

#pragma pack(pop)
}}
