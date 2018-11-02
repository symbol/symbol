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
#include "catapult/types.h"
#include <memory>
#include <vector>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace model { struct Transaction; }
}

namespace catapult { namespace test {

	// notice that these helper functions create concrete transaction types
	// they are in local test utils because non-local tests should be using MockTransaction

	/// Creates basic unsigned transfer transaction with specified \a signerPublicKey, \a recipient and \a amount.
	std::unique_ptr<model::Transaction> CreateUnsignedTransferTransaction(
			const Key& signerPublicKey,
			const UnresolvedAddress& recipient,
			Amount amount);

	/// Creates basic signed transfer transaction with \a signer, \a recipient and \a amount.
	std::unique_ptr<model::Transaction> CreateTransferTransaction(
			const crypto::KeyPair& signer,
			const UnresolvedAddress& recipient,
			Amount amount);

	/// Creates basic signed transfer transaction with \a signer, \a recipientPublicKey and \a amount.
	std::unique_ptr<model::Transaction> CreateTransferTransaction(
			const crypto::KeyPair& signer,
			const Key& recipientPublicKey,
			Amount amount);

	/// Creates signed root register namespace transaction with \a signer, \a name and \a duration.
	std::unique_ptr<model::Transaction> CreateRegisterRootNamespaceTransaction(
			const crypto::KeyPair& signer,
			const std::string& name,
			BlockDuration duration);
}}
