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

	/// Transactions builder and generator for transfer and account restriction transactions.
	class AccountRestrictionTransactionsBuilder : public BasicTransactionsBuilder {
	private:
		// region descriptors

		struct AccountAddressRestrictionBlockDescriptor {
			size_t SenderId;
			size_t PartnerId;
			bool IsAdd;
		};

		// endregion

	public:
		/// Creates a builder around \a accounts.
		explicit AccountRestrictionTransactionsBuilder(const Accounts& accounts);

	private:
		// BasicTransactionsBuilder
		std::unique_ptr<model::Transaction> generate(
				uint32_t descriptorType,
				const std::shared_ptr<const void>& pDescriptor,
				Timestamp deadline) const override;

	public:
		/// Adds an account address restriction that blocks \a partnerId from sending to \a senderId.
		void addAccountAddressRestrictionBlock(size_t senderId, size_t partnerId);

		/// Adds an account address restriction that unblocks \a partnerId from sending to \a senderId.
		void delAccountAddressRestrictionBlock(size_t senderId, size_t partnerId);

	private:
		std::unique_ptr<model::Transaction> createAddressRestrictionTransaction(
				const AccountAddressRestrictionBlockDescriptor& descriptor,
				Timestamp deadline) const;

	private:
		enum class DescriptorType { Account_Restriction_Address_Block = 1 };
	};
}}
