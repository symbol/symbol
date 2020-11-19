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

#include "TransactionsBuilder.h"
#include "tests/test/local/RealTransactionFactory.h"

namespace catapult { namespace test {

	// region ctor

	TransactionsBuilder::TransactionsBuilder(const Accounts& accounts) : BasicTransactionsBuilder(accounts)
	{}

	// endregion

	// region generate

	std::unique_ptr<model::Transaction> TransactionsBuilder::generate(
			uint32_t descriptorType,
			const std::shared_ptr<const void>& pDescriptor,
			Timestamp deadline) const {
		switch (static_cast<DescriptorType>(descriptorType)) {
		case DescriptorType::Namespace:
			return createNamespaceRegistration(CastToDescriptor<NamespaceDescriptor>(pDescriptor), deadline);

		case DescriptorType::Alias:
			return createAddressAlias(CastToDescriptor<NamespaceDescriptor>(pDescriptor), deadline);
		}

		return nullptr;
	}

	// endregion

	// region add / create

	void TransactionsBuilder::addNamespace(size_t ownerId, const std::string& name, BlockDuration duration, size_t aliasId) {
		auto descriptor = NamespaceDescriptor{ ownerId, name, duration, aliasId };
		add(DescriptorType::Namespace, descriptor);

		if (0 != descriptor.AddressAliasId)
			add(DescriptorType::Alias, descriptor);
	}

	std::unique_ptr<model::Transaction> TransactionsBuilder::createNamespaceRegistration(
			const NamespaceDescriptor& descriptor,
			Timestamp deadline) const {
		const auto& ownerKeyPair = accounts().getKeyPair(descriptor.OwnerId);

		auto pTransaction = CreateRootNamespaceRegistrationTransaction(ownerKeyPair, descriptor.Name, descriptor.Duration);
		return SignWithDeadline(std::move(pTransaction), ownerKeyPair, deadline);
	}

	std::unique_ptr<model::Transaction> TransactionsBuilder::createAddressAlias(
			const NamespaceDescriptor& descriptor,
			Timestamp deadline) const {
		const auto& ownerKeyPair = accounts().getKeyPair(descriptor.OwnerId);
		const auto& aliasedAddress = accounts().getAddress(descriptor.AddressAliasId);

		auto pTransaction = CreateRootAddressAliasTransaction(ownerKeyPair, descriptor.Name, aliasedAddress);
		return SignWithDeadline(std::move(pTransaction), ownerKeyPair, deadline);
	}

	// endregion
}}
