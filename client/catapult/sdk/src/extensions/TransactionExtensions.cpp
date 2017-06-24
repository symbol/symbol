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
