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

#include "TransactionExtensions.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/Signer.h"

namespace catapult { namespace extensions {

	namespace {
		RawBuffer TransactionDataBuffer(const model::Transaction& transaction) {
			return {
				reinterpret_cast<const uint8_t*>(&transaction) + model::VerifiableEntity::Header_Size,
				transaction.Size - model::VerifiableEntity::Header_Size
			};
		}
	}

	void SignTransaction(const crypto::KeyPair& signer, model::Transaction& transaction) {
		crypto::Sign(signer, TransactionDataBuffer(transaction), transaction.Signature);
	}

	bool VerifyTransactionSignature(const model::Transaction& transaction) {
		return crypto::Verify(transaction.Signer, TransactionDataBuffer(transaction), transaction.Signature);
	}
}}
