#include "TransactionTestUtils.h"
#include "sdk/src/builders/MosaicDefinitionBuilder.h"
#include "sdk/src/builders/RegisterNamespaceBuilder.h"
#include "tests/test/core/AddressTestUtils.h"

namespace catapult { namespace test {

	namespace {
		constexpr auto Network_Identifier = model::NetworkIdentifier::Mijin_Test;

		std::string GenerateValidName(size_t size) {
			static constexpr char Valid_Alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";

			std::string name(size, '\0');
			std::generate(name.begin(), name.end(), []() {
				return Valid_Alphabet[test::Random() % (CountOf(Valid_Alphabet) - 1)];
			});
			return name;
		}

		class TestMosaicDefinitionBuilder : public builders::MosaicDefinitionBuilder {
		public:
			TestMosaicDefinitionBuilder(const Key& signer, NamespaceId parentId, const RawString& name)
					: MosaicDefinitionBuilder(Network_Identifier, signer, parentId, name)
			{}

		public:
			void setOptionalProperties(const std::initializer_list<model::MosaicProperty>& optionalProperties) {
				for (const auto& property : optionalProperties)
					addOptionalProperty(property.Id, property.Value);
			}
		};

		auto CreateTransaction(builders::MosaicDefinitionBuilder& builder) {
			auto pTransaction = builder.build();
			pTransaction->Fee = test::GenerateRandomValue<Amount>();
			pTransaction->Deadline = test::GenerateRandomValue<Timestamp>();
			return pTransaction;
		}
	}

	// region MosaicDefinitionTransactionFactory

	using MosaicDefinitionTxPointer = MosaicDefinitionTransactionFactory::TransactionPointerType;

	MosaicDefinitionTxPointer MosaicDefinitionTransactionFactory::CreateUnsigned(
			const Key& signerPublicKey,
			const std::string& name,
			NamespaceId parentId,
			ArtifactDuration duration) {
		TestMosaicDefinitionBuilder builder(signerPublicKey, parentId, name);
		builder.setDuration(duration);

		return CreateTransaction(builder);
	}

	MosaicDefinitionTxPointer MosaicDefinitionTransactionFactory::GenerateRandomUnsigned(size_t nameSize) {
		return GenerateRandomUnsigned(nameSize, test::GenerateRandomValue<ArtifactDuration>());
	}

	MosaicDefinitionTxPointer MosaicDefinitionTransactionFactory::GenerateRandomUnsigned(
			size_t nameSize,
			ArtifactDuration duration) {
		return GenerateRandomUnsignedWithName(GenerateValidName(nameSize), duration);
	}

	MosaicDefinitionTxPointer MosaicDefinitionTransactionFactory::GenerateRandomUnsignedWithName(const std::string& name) {
		return GenerateRandomUnsignedWithName(name, test::GenerateRandomValue<ArtifactDuration>());
	}

	MosaicDefinitionTxPointer MosaicDefinitionTransactionFactory::GenerateRandomUnsignedWithProperties(
			model::MosaicFlags flags,
			uint8_t divisibility,
			std::initializer_list<model::MosaicProperty> optionalProperties) {
		auto signerPublicKey = test::GenerateRandomData<Key_Size>();
		auto parentId = test::GenerateRandomValue<NamespaceId>();
		auto name = GenerateValidName(123);
		TestMosaicDefinitionBuilder builder(signerPublicKey, parentId, name);
		builder.setOptionalProperties(optionalProperties);

		auto pTransaction = CreateTransaction(builder);
		pTransaction->PropertiesHeader.Flags = flags;
		pTransaction->PropertiesHeader.Divisibility = divisibility;
		return pTransaction;
	}

	MosaicDefinitionTxPointer MosaicDefinitionTransactionFactory::GenerateRandomUnsignedWithName(
			const std::string& name,
			ArtifactDuration duration) {
		auto signerPublicKey = test::GenerateRandomData<Key_Size>();
		auto pTransaction = CreateUnsigned(
				signerPublicKey,
				name,
				test::GenerateRandomValue<NamespaceId>(),
				duration);
		FillWithRandomData(pTransaction->Signature);
		return pTransaction;
	}

	// endregion

	// region RegisterNamespaceTransactionFactory

	using RegisterNamespaceTxPointer = RegisterNamespaceTransactionFactory::TransactionPointerType;

	RegisterNamespaceTxPointer RegisterNamespaceTransactionFactory::CreateUnsigned(
			const Key& signerPublicKey,
			NamespaceId parentId,
			const std::string& name,
			ArtifactDuration duration) {
		builders::RegisterNamespaceBuilder builder(Network_Identifier, signerPublicKey, name);

		if (Namespace_Base_Id != parentId)
			builder.setParentId(parentId);
		else
			builder.setDuration(duration);

		auto pTransaction = builder.build();
		pTransaction->Fee = test::GenerateRandomValue<Amount>();
		pTransaction->Deadline = test::GenerateRandomValue<Timestamp>();
		return pTransaction;
	}

	RegisterNamespaceTxPointer RegisterNamespaceTransactionFactory::GenerateRandomUnsigned(size_t nameSize) {
		return GenerateRandomUnsigned(test::GenerateRandomValue<NamespaceId>(), nameSize);
	}

	RegisterNamespaceTxPointer RegisterNamespaceTransactionFactory::GenerateRandomUnsigned(NamespaceId parentId, size_t nameSize) {
		return GenerateRandomUnsignedWithName(parentId, GenerateValidName(nameSize));
	}

	RegisterNamespaceTxPointer RegisterNamespaceTransactionFactory::GenerateRandomUnsignedWithName(const std::string& name) {
		return GenerateRandomUnsignedWithName(test::GenerateRandomValue<NamespaceId>(), name);
	}

	RegisterNamespaceTxPointer RegisterNamespaceTransactionFactory::GenerateRandomUnsignedWithName(
			NamespaceId parentId,
			const std::string& name) {
		auto signerPublicKey = test::GenerateRandomData<Key_Size>();
		auto pTransaction = CreateUnsigned(
				signerPublicKey,
				parentId,
				name,
				test::GenerateRandomValue<ArtifactDuration>());
		FillWithRandomData(pTransaction->Signature);
		return pTransaction;
	}

	// endregion
}}
