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

#include "TransactionExtensions.h"
#include "plugins/txes/aggregate/src/model/AggregateTransaction.h"
#include "catapult/crypto/Signer.h"
#include "catapult/model/EntityHasher.h"

namespace catapult { namespace extensions {

	namespace {
		constexpr bool IsAggregateType(model::EntityType entityType) {
			return model::Entity_Type_Aggregate_Bonded == entityType || model::Entity_Type_Aggregate_Complete == entityType;
		}

		RawBuffer TransactionDataBuffer(const model::Transaction& transaction) {
			const auto* pData = reinterpret_cast<const uint8_t*>(&transaction) + model::Transaction::Header_Size;
			size_t size = IsAggregateType(transaction.Type)
					? sizeof(model::AggregateTransaction) - model::Transaction::Header_Size - model::AggregateTransaction::Footer_Size
					: transaction.Size - model::Transaction::Header_Size;
			return { pData, size };
		}
	}

	TransactionExtensions::TransactionExtensions(const GenerationHashSeed& generationHashSeed) : m_generationHashSeed(generationHashSeed)
	{}

	Hash256 TransactionExtensions::hash(const model::Transaction& transaction) const {
		return model::CalculateHash(transaction, m_generationHashSeed, TransactionDataBuffer(transaction));
	}

	void TransactionExtensions::sign(const crypto::KeyPair& signer, model::Transaction& transaction) const {
		crypto::Sign(signer, { m_generationHashSeed, TransactionDataBuffer(transaction) }, transaction.Signature);
	}

	bool TransactionExtensions::verify(const model::Transaction& transaction) const {
		return crypto::Verify(
				transaction.SignerPublicKey,
				{ m_generationHashSeed, TransactionDataBuffer(transaction) },
				transaction.Signature);
	}
}}
