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
#include "Accounts.h"
#include "TransactionsGenerator.h"
#include <vector>

namespace catapult { namespace test {

	/// Basic transactions builder and generator for transfer transactions.
	class BasicTransactionsBuilder : public TransactionsGenerator {
	private:
		// region descriptors

		struct TransferDescriptor {
			size_t SenderId;
			size_t RecipientId;
			catapult::Amount Amount;
			std::string RecipientAlias; // optional
		};

		// endregion

	public:
		/// Creates a builder around \a accounts.
		explicit BasicTransactionsBuilder(const Accounts& accounts);

	public:
		// TransactionsGenerator
		size_t size() const override;
		std::unique_ptr<model::Transaction> generateAt(size_t index, Timestamp deadline) const override;

	public:
		/// Adds a transfer from \a senderId to \a recipientId for amount \a transferAmount.
		void addTransfer(size_t senderId, size_t recipientId, Amount transferAmount);

		/// Adds a transfer from \a senderId to \a recipientAlias for amount \a transferAmount.
		void addTransfer(size_t senderId, const std::string& recipientAlias, Amount transferAmount);

	protected:
		/// Gets the accounts.
		const Accounts& accounts() const;

		/// Gets the transaction descriptor pair at \ index.
		const std::pair<uint32_t, std::shared_ptr<const void>>& getAt(size_t index) const;

		/// Adds transaction described by \a descriptor with descriptor type (\a descriptorType).
		template<typename TDescriptorType, typename TDescriptor>
		void add(TDescriptorType descriptorType, const TDescriptor& descriptor) {
			m_transactionDescriptorPairs.emplace_back(
					utils::to_underlying_type(descriptorType),
					std::make_shared<TDescriptor>(descriptor));
		}

	private:
		/// Generates transaction with specified \a deadline given descriptor (\a pDescriptor) and descriptor type (\a descriptorType).
		virtual std::unique_ptr<model::Transaction> generate(
				uint32_t descriptorType,
				const std::shared_ptr<const void>& pDescriptor,
				Timestamp deadline) const = 0;

	private:
		std::unique_ptr<model::Transaction> createTransfer(const TransferDescriptor& descriptor, Timestamp deadline) const;

	protected:
		/// Casts \a pVoid to a descriptor.
		template<typename TDescriptor>
		static const TDescriptor& CastToDescriptor(const std::shared_ptr<const void>& pVoid) {
			return *static_cast<const TDescriptor*>(pVoid.get());
		}

		/// Signs \a pTransaction with \a signerKeyPair and sets specified transaction \a deadline.
		static std::unique_ptr<model::Transaction> SignWithDeadline(
				std::unique_ptr<model::Transaction>&& pTransaction,
				const crypto::KeyPair& signerKeyPair,
				Timestamp deadline);

	private:
		enum class DescriptorType { Transfer };

	private:
		const Accounts& m_accounts;

		std::vector<std::pair<uint32_t, std::shared_ptr<const void>>> m_transactionDescriptorPairs;
	};
}}
