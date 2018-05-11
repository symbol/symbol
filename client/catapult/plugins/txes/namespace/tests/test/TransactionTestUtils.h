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
#include "sdk/src/builders/MosaicDefinitionBuilder.h"
#include "src/model/MosaicDefinitionTransaction.h"
#include "src/model/RegisterNamespaceTransaction.h"

namespace catapult { namespace test {

	/// Generates valid namespace name of length \a size.
	std::string GenerateValidName(size_t size);

	// region MosaicDefinitionTransactionFactory

	/// Factory for creating mosaic definition transactions.
	struct MosaicDefinitionTransactionFactory {
	public:
		using TransactionPointerType = std::unique_ptr<model::MosaicDefinitionTransaction>;

	public:
		/// Creates mosaic definition transaction with specified \a signerPublicKey, \a name, \a parentId and \a duration.
		static TransactionPointerType CreateUnsigned(
				const Key& signerPublicKey,
				const std::string& name,
				NamespaceId parentId,
				BlockDuration duration);

		/// Generates a random mosaic definition transaction with name of given size (\a nameSize).
		static TransactionPointerType GenerateRandomUnsigned(size_t nameSize);

		/// Generates a random mosaic definition transaction with name of given size (\a nameSize) and \a duration.
		static TransactionPointerType GenerateRandomUnsigned(size_t nameSize, BlockDuration duration);

		/// Generates a random mosaic definition transaction with given \a name.
		static TransactionPointerType GenerateRandomUnsignedWithName(const std::string& name);

		/// Generates a random mosaic definition transaction with given properties: \a flags, \a divisibility
		/// and optional properties (\a optionalProperties).
		static TransactionPointerType GenerateRandomUnsignedWithProperties(
				model::MosaicFlags flags,
				uint8_t divisibility,
				std::initializer_list<model::MosaicProperty> optionalProperties);

	private:
		static TransactionPointerType GenerateRandomUnsignedWithName(const std::string& name, BlockDuration duration);
	};

	// endregion

	// region RegisterNamespaceTransactionFactory

	/// Factory for creating register namespace transactions.
	struct RegisterNamespaceTransactionFactory {
	public:
		using TransactionPointerType = std::unique_ptr<model::RegisterNamespaceTransaction>;

	public:
		/// Creates register namespace transaction with specified \a signerPublicKey, \a parentId, \a name and \a duration.
		static TransactionPointerType CreateUnsigned(
				const Key& signerPublicKey,
				NamespaceId parentId,
				const std::string& name,
				BlockDuration duration);

		/// Generates a random register namespace transaction with name of given size (\a nameSize).
		static TransactionPointerType GenerateRandomUnsigned(size_t nameSize);

		/// Generates a random register namespace transaction with \a parentId and name of given size (\a nameSize).
		static TransactionPointerType GenerateRandomUnsigned(NamespaceId parentId, size_t nameSize);

		/// Generates a random register namespace transaction with given \a name.
		static TransactionPointerType GenerateRandomUnsignedWithName(const std::string& name);

		/// Generates a random register namespace transaction with \a parentId and given \a name.
		static TransactionPointerType GenerateRandomUnsignedWithName(NamespaceId parentId, const std::string& name);
	};

	// endregion
}}
