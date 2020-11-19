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
#include "BasicTransactionsBuilder.h"

namespace catapult { namespace test {

	/// Transactions builder and generator for transfer and secret lock transactions.
	class SecretLockTransactionsBuilder : public BasicTransactionsBuilder {
	private:
		// region descriptors

		struct SecretLockDescriptor {
			size_t SenderId;
			size_t RecipientId;
			catapult::Amount Amount;
			BlockDuration Duration;
			Hash256 Secret;
		};

		struct SecretProofDescriptor {
			size_t SenderId;
			size_t RecipientId;
			std::vector<uint8_t> Proof;
		};

		// endregion

	public:
		/// Creates a builder around \a accounts.
		explicit SecretLockTransactionsBuilder(const Accounts& accounts);

	private:
		// BasicTransactionsBuilder
		std::unique_ptr<model::Transaction> generate(
				uint32_t descriptorType,
				const std::shared_ptr<const void>& pDescriptor,
				Timestamp deadline) const override;

	public:
		/// Adds a secret lock from \a senderId to \a recipientId for amount \a transferAmount, specified \a duration and \a proof.
		std::vector<uint8_t> addSecretLock(
				size_t senderId,
				size_t recipientId,
				Amount transferAmount,
				BlockDuration duration,
				const std::vector<uint8_t>& proof);

		/// Adds a secret lock from \a senderId to \a recipientId for amount \a transferAmount and specified \a duration.
		std::vector<uint8_t> addSecretLock(size_t senderId, size_t recipientId, Amount transferAmount, BlockDuration duration);

		/// Adds a secret proof from \a senderId to \a recipientId using \a proof data.
		void addSecretProof(size_t senderId, size_t recipientId, const std::vector<uint8_t>& proof);

	private:
		std::unique_ptr<model::Transaction> createSecretLock(const SecretLockDescriptor& descriptor, Timestamp deadline) const;

		std::unique_ptr<model::Transaction> createSecretProof(const SecretProofDescriptor& descriptor, Timestamp deadline) const;

	private:
		enum class DescriptorType { Secret_Lock = 1, Secret_Proof };
	};
}}
