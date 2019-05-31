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
#include "BasicTransactionsBuilder.h"

namespace catapult { namespace test {

	/// Transactions builder and generator for transfer and property transactions.
	class PropertyTransactionsBuilder : public BasicTransactionsBuilder {
	private:
		// region descriptors

		struct PropertyAddressBlockDescriptor {
			size_t SenderId;
			size_t PartnerId;
			bool IsAdd;
		};

		// endregion

	public:
		/// Creates a builder around \a accounts.
		explicit PropertyTransactionsBuilder(const Accounts& accounts);

	private:
		// BasicTransactionsBuilder
		std::unique_ptr<model::Transaction> generate(
				uint32_t descriptorType,
				const std::shared_ptr<const void>& pDescriptor,
				Timestamp deadline) const override;

	public:
		/// Adds a property that blocks \a partnerId from sending to \a senderId.
		void addAddressBlockProperty(size_t senderId, size_t partnerId);

		/// Adds a property that unblocks \a partnerId from sending to \a senderId.
		void addAddressUnblockProperty(size_t senderId, size_t partnerId);

	private:
		std::unique_ptr<model::Transaction> createAddressPropertyTransaction(
				const PropertyAddressBlockDescriptor& descriptor,
				Timestamp deadline) const;

	private:
		enum class DescriptorType { Property_Address_Block = 1 };
	};
}}
