#pragma once
#include "sdk/src/builders/MosaicDefinitionBuilder.h"
#include "src/model/MosaicDefinitionTransaction.h"
#include "src/model/RegisterNamespaceTransaction.h"

namespace catapult { namespace test {

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
				ArtifactDuration duration);

		/// Generates a random mosaic definition transaction with name of given size (\a nameSize).
		static TransactionPointerType GenerateRandomUnsigned(size_t nameSize);

		/// Generates a random mosaic definition transaction with name of given size (\a nameSize) and \a duration.
		static TransactionPointerType GenerateRandomUnsigned(size_t nameSize, ArtifactDuration duration);

		/// Generates a random mosaic definition transaction with given \a name.
		static TransactionPointerType GenerateRandomUnsignedWithName(const std::string& name);

		/// Generates a random mosaic definition transaction with given properties: \a flags, \a divisibility
		/// and optional properties (\a optionalProperties).
		static TransactionPointerType GenerateRandomUnsignedWithProperties(
				model::MosaicFlags flags,
				uint8_t divisibility,
				std::initializer_list<model::MosaicProperty> optionalProperties);

	private:
		static TransactionPointerType GenerateRandomUnsignedWithName(const std::string& name, ArtifactDuration duration);
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
				ArtifactDuration duration);

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
