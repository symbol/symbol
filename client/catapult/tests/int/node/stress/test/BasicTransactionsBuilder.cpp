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

#include "BasicTransactionsBuilder.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "sdk/src/extensions/TransactionExtensions.h"
#include "plugins/txes/namespace/src/model/NamespaceIdGenerator.h"
#include "tests/test/local/RealTransactionFactory.h"
#include "tests/test/nodeps/Nemesis.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
	}

	// region ctor

	BasicTransactionsBuilder::BasicTransactionsBuilder(const Accounts& accounts) : m_accounts(accounts)
	{}

	// endregion

	// region TransactionsGenerator

	size_t BasicTransactionsBuilder::size() const {
		return m_transactionDescriptorPairs.size();
	}

	std::unique_ptr<model::Transaction> BasicTransactionsBuilder::generateAt(size_t index, Timestamp deadline) const {
		const auto& pair = m_transactionDescriptorPairs[index];
		auto pTransaction = generate(pair.first, pair.second, deadline);
		if (pTransaction)
			return pTransaction;

		if (DescriptorType::Transfer == static_cast<DescriptorType>(pair.first))
			return createTransfer(CastToDescriptor<TransferDescriptor>(pair.second), deadline);

		CATAPULT_THROW_INVALID_ARGUMENT_1("cannot generate unknown transaction type", pair.first);
	}

	// endregion

	// region add

	void BasicTransactionsBuilder::addTransfer(size_t senderId, size_t recipientId, Amount transferAmount) {
		auto descriptor = TransferDescriptor{ senderId, recipientId, transferAmount, "" };
		add(DescriptorType::Transfer, descriptor);
	}

	void BasicTransactionsBuilder::addTransfer(size_t senderId, const std::string& recipientAlias, Amount transferAmount) {
		auto descriptor = TransferDescriptor{ senderId, 0, transferAmount, recipientAlias };
		add(DescriptorType::Transfer, descriptor);
	}

	// endregion

	// region protected

	const Accounts& BasicTransactionsBuilder::accounts() const {
		return m_accounts;
	}

	const std::pair<uint32_t, std::shared_ptr<const void>>& BasicTransactionsBuilder::getAt(size_t index) const {
		return m_transactionDescriptorPairs[index];
	}

	// endregion

	// region create

	namespace {
		UnresolvedAddress RootAliasToAddress(const std::string& namespaceName) {
			auto namespaceId = model::GenerateNamespaceId(NamespaceId(), namespaceName);

			UnresolvedAddress address{}; // force zero initialization
			address[0] = utils::to_underlying_type(Network_Identifier) | 0x01;
			std::memcpy(address.data() + 1, &namespaceId, sizeof(NamespaceId));
			return address;
		}
	}

	std::unique_ptr<model::Transaction> BasicTransactionsBuilder::createTransfer(
			const TransferDescriptor& descriptor,
			Timestamp deadline) const {
		const auto& senderKeyPair = m_accounts.getKeyPair(descriptor.SenderId);
		auto recipientAddress = descriptor.RecipientAlias.empty()
				? extensions::CopyToUnresolvedAddress(m_accounts.getAddress(descriptor.RecipientId))
				: RootAliasToAddress(descriptor.RecipientAlias);

		auto pTransaction = CreateTransferTransaction(senderKeyPair, recipientAddress, descriptor.Amount);
		return SignWithDeadline(std::move(pTransaction), senderKeyPair, deadline);
	}

	// endregion

	std::unique_ptr<model::Transaction> BasicTransactionsBuilder::SignWithDeadline(
			std::unique_ptr<model::Transaction>&& pTransaction,
			const crypto::KeyPair& signerKeyPair,
			Timestamp deadline) {
		pTransaction->Deadline = deadline;
		pTransaction->MaxFee = Amount(pTransaction->Size);
		extensions::TransactionExtensions(GetNemesisGenerationHashSeed()).sign(signerKeyPair, *pTransaction);
		return std::move(pTransaction);
	}
}}
