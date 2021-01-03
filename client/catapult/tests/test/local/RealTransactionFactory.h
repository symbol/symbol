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
#include "catapult/crypto/KeyPair.h"
#include "catapult/types.h"
#include <memory>
#include <vector>

namespace catapult { namespace model { struct Transaction; } }

namespace catapult { namespace test {

	// notice that these helper functions create concrete transaction types
	// they are in local test utils because non-local tests should be using MockTransaction

	/// Gets the size of a transfer transaction without a message.
	size_t GetTransferTransactionSize();

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

	/// Creates signed root namespace registration transaction with \a signer, \a name and \a duration.
	std::unique_ptr<model::Transaction> CreateRootNamespaceRegistrationTransaction(
			const crypto::KeyPair& signer,
			const std::string& name,
			BlockDuration duration);

	/// Creates a signed root address alias transaction with \a signer, root namespace \a name and \a address.
	std::unique_ptr<model::Transaction> CreateRootAddressAliasTransaction(
			const crypto::KeyPair& signer,
			const std::string& name,
			const Address& address);
}}
