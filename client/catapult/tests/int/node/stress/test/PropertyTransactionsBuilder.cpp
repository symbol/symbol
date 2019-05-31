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

#include "PropertyTransactionsBuilder.h"
#include "sdk/src/builders/AddressPropertyBuilder.h"
#include "sdk/src/extensions/ConversionExtensions.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;
	}

	// region ctor

	PropertyTransactionsBuilder::PropertyTransactionsBuilder(const Accounts& accounts) : BasicTransactionsBuilder(accounts)
	{}

	// endregion

	// region generate

	std::unique_ptr<model::Transaction> PropertyTransactionsBuilder::generate(
			uint32_t descriptorType,
			const std::shared_ptr<const void>& pDescriptor,
			Timestamp deadline) const {
		switch (static_cast<DescriptorType>(descriptorType)) {
		case DescriptorType::Property_Address_Block:
			return createAddressPropertyTransaction(CastToDescriptor<PropertyAddressBlockDescriptor>(pDescriptor), deadline);
		}

		return nullptr;
	}

	// endregion

	// region add / create

	void PropertyTransactionsBuilder::addAddressBlockProperty(size_t senderId, size_t partnerId) {
		auto descriptor = PropertyAddressBlockDescriptor{ senderId, partnerId, true };
		add(DescriptorType::Property_Address_Block, descriptor);
	}

	void PropertyTransactionsBuilder::addAddressUnblockProperty(size_t senderId, size_t partnerId) {
		auto descriptor = PropertyAddressBlockDescriptor{ senderId, partnerId, false };
		add(DescriptorType::Property_Address_Block, descriptor);
	}

	std::unique_ptr<model::Transaction> PropertyTransactionsBuilder::createAddressPropertyTransaction(
			const PropertyAddressBlockDescriptor& descriptor,
			Timestamp deadline) const {
		const auto& senderKeyPair = accounts().getKeyPair(descriptor.SenderId);
		auto partnerAddress = extensions::CopyToUnresolvedAddress(accounts().getAddress(descriptor.PartnerId));

		builders::AddressPropertyBuilder builder(Network_Identifier, senderKeyPair.publicKey());
		builder.setPropertyType(model::PropertyType::Block | model::PropertyType::Address);
		builder.addModification({
			descriptor.IsAdd ? model::PropertyModificationType::Add : model::PropertyModificationType::Del,
			partnerAddress
		});
		auto pTransaction = builder.build();

		return SignWithDeadline(std::move(pTransaction), senderKeyPair, deadline);
	}

	// endregion
}}
