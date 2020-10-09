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

#include "AccountRestrictionTransactionsBuilder.h"
#include "sdk/src/builders/AccountAddressRestrictionBuilder.h"
#include "sdk/src/extensions/ConversionExtensions.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
	}

	// region ctor

	AccountRestrictionTransactionsBuilder::AccountRestrictionTransactionsBuilder(const Accounts& accounts)
			: BasicTransactionsBuilder(accounts)
	{}

	// endregion

	// region generate

	std::unique_ptr<model::Transaction> AccountRestrictionTransactionsBuilder::generate(
			uint32_t descriptorType,
			const std::shared_ptr<const void>& pDescriptor,
			Timestamp deadline) const {
		switch (static_cast<DescriptorType>(descriptorType)) {
		case DescriptorType::Account_Restriction_Address_Block:
			return createAddressRestrictionTransaction(CastToDescriptor<AccountAddressRestrictionBlockDescriptor>(pDescriptor), deadline);
		}

		return nullptr;
	}

	// endregion

	// region add / create

	void AccountRestrictionTransactionsBuilder::addAccountAddressRestrictionBlock(size_t senderId, size_t partnerId) {
		auto descriptor = AccountAddressRestrictionBlockDescriptor{ senderId, partnerId, true };
		add(DescriptorType::Account_Restriction_Address_Block, descriptor);
	}

	void AccountRestrictionTransactionsBuilder::delAccountAddressRestrictionBlock(size_t senderId, size_t partnerId) {
		auto descriptor = AccountAddressRestrictionBlockDescriptor{ senderId, partnerId, false };
		add(DescriptorType::Account_Restriction_Address_Block, descriptor);
	}

	std::unique_ptr<model::Transaction> AccountRestrictionTransactionsBuilder::createAddressRestrictionTransaction(
			const AccountAddressRestrictionBlockDescriptor& descriptor,
			Timestamp deadline) const {
		const auto& senderKeyPair = accounts().getKeyPair(descriptor.SenderId);
		auto partnerAddress = extensions::CopyToUnresolvedAddress(accounts().getAddress(descriptor.PartnerId));

		builders::AccountAddressRestrictionBuilder builder(Network_Identifier, senderKeyPair.publicKey());
		builder.setRestrictionFlags(model::AccountRestrictionFlags::Block | model::AccountRestrictionFlags::Address);
		if (descriptor.IsAdd)
			builder.addRestrictionAddition(partnerAddress);
		else
			builder.addRestrictionDeletion(partnerAddress);

		auto pTransaction = builder.build();
		return SignWithDeadline(std::move(pTransaction), senderKeyPair, deadline);
	}

	// endregion
}}
