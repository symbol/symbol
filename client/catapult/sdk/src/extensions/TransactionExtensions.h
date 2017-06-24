#pragma once
#include "catapult/model/Transaction.h"

namespace catapult { namespace extensions {

	/// Signs the \a transaction using \a signer private key.
	void SignTransaction(const crypto::KeyPair& signer, model::Transaction& transaction);

	/// Verifies signature of the \a transaction.
	bool VerifyTransactionSignature(const model::Transaction& transaction);
}}
