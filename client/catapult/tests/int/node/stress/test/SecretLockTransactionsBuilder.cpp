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

#include "SecretLockTransactionsBuilder.h"
#include "sdk/src/builders/SecretLockBuilder.h"
#include "sdk/src/builders/SecretProofBuilder.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/nodeps/Random.h"
#include "tests/test/nodeps/TestConstants.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
	}

	// region ctor

	SecretLockTransactionsBuilder::SecretLockTransactionsBuilder(const Accounts& accounts) : BasicTransactionsBuilder(accounts)
	{}

	// endregion

	// region generate

	std::unique_ptr<model::Transaction> SecretLockTransactionsBuilder::generate(
			uint32_t descriptorType,
			const std::shared_ptr<const void>& pDescriptor,
			Timestamp deadline) const {
		switch (static_cast<DescriptorType>(descriptorType)) {
		case DescriptorType::Secret_Lock:
			return createSecretLock(CastToDescriptor<SecretLockDescriptor>(pDescriptor), deadline);

		case DescriptorType::Secret_Proof:
			return createSecretProof(CastToDescriptor<SecretProofDescriptor>(pDescriptor), deadline);
		}

		return nullptr;
	}

	// endregion

	// region add / create

	std::vector<uint8_t> SecretLockTransactionsBuilder::addSecretLock(
			size_t senderId,
			size_t recipientId,
			Amount transferAmount,
			BlockDuration duration,
			const std::vector<uint8_t>& proof) {
		Hash256 secret;
		crypto::Sha3_256(proof, secret);

		auto descriptor = SecretLockDescriptor{ senderId, recipientId, transferAmount, duration, secret };
		add(DescriptorType::Secret_Lock, descriptor);
		return proof;
	}

	std::vector<uint8_t> SecretLockTransactionsBuilder::addSecretLock(
			size_t senderId,
			size_t recipientId,
			Amount transferAmount,
			BlockDuration duration) {
		return addSecretLock(senderId, recipientId, transferAmount, duration, test::GenerateRandomVector(25));
	}

	void SecretLockTransactionsBuilder::addSecretProof(size_t senderId, size_t recipientId, const std::vector<uint8_t>& proof) {
		auto descriptor = SecretProofDescriptor{ senderId, recipientId, proof };
		add(DescriptorType::Secret_Proof, descriptor);
	}

	std::unique_ptr<model::Transaction> SecretLockTransactionsBuilder::createSecretLock(
			const SecretLockDescriptor& descriptor,
			Timestamp deadline) const {
		const auto& senderKeyPair = accounts().getKeyPair(descriptor.SenderId);
		auto recipientAddress = extensions::CopyToUnresolvedAddress(accounts().getAddress(descriptor.RecipientId));

		builders::SecretLockBuilder builder(Network_Identifier, senderKeyPair.publicKey());
		builder.setMosaic({ extensions::CastToUnresolvedMosaicId(Default_Currency_Mosaic_Id), descriptor.Amount });
		builder.setDuration(descriptor.Duration);
		builder.setHashAlgorithm(model::LockHashAlgorithm::Op_Sha3_256);
		builder.setSecret(descriptor.Secret);
		builder.setRecipientAddress(recipientAddress);
		auto pTransaction = builder.build();

		return SignWithDeadline(std::move(pTransaction), senderKeyPair, deadline);
	}

	std::unique_ptr<model::Transaction> SecretLockTransactionsBuilder::createSecretProof(
			const SecretProofDescriptor& descriptor,
			Timestamp deadline) const {
		const auto& senderKeyPair = accounts().getKeyPair(descriptor.SenderId);
		auto recipientAddress = extensions::CopyToUnresolvedAddress(accounts().getAddress(descriptor.RecipientId));

		Hash256 secret;
		crypto::Sha3_256(descriptor.Proof, secret);

		builders::SecretProofBuilder builder(Network_Identifier, senderKeyPair.publicKey());
		builder.setHashAlgorithm(model::LockHashAlgorithm::Op_Sha3_256);
		builder.setSecret(secret);
		builder.setRecipientAddress(recipientAddress);
		builder.setProof(descriptor.Proof);
		auto pTransaction = builder.build();

		return SignWithDeadline(std::move(pTransaction), senderKeyPair, deadline);
	}

	// endregion
}}
