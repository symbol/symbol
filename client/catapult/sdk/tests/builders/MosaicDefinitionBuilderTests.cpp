#include "src/builders/MosaicDefinitionBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "tests/TestHarness.h"

#define TEST_CLASS MosaicDefinitionBuilderTests

namespace catapult { namespace builders {

	namespace {
		auto CreateBuilderWithName(const RawString& name) {
			return MosaicDefinitionBuilder(
				static_cast<model::NetworkIdentifier>(0x62),
				test::GenerateRandomData<Key_Size>(),
				test::GenerateRandomValue<NamespaceId>(),
				name);
		}

		void AssertCanBuildTransaction(
				NamespaceId parentNamespaceId,
				const std::string& mosaicName,
				size_t propertiesSize,
				const std::function<void (MosaicDefinitionBuilder&)>& buildTransaction,
				const std::function<void (const model::MosaicDefinitionTransaction&)>& validateTransaction) {

			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			MosaicDefinitionBuilder builder(networkId, signer, parentNamespaceId, mosaicName);
			buildTransaction(builder);
			auto pTransaction = builder.build();

			// Assert:
			ASSERT_EQ(sizeof(model::MosaicDefinitionTransaction) + mosaicName.size() + propertiesSize, pTransaction->Size);
			EXPECT_EQ(Signature{}, pTransaction->Signature);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(model::EntityType::Mosaic_Definition, pTransaction->Type);
			EXPECT_EQ(parentNamespaceId, pTransaction->ParentId);

			EXPECT_EQ(Amount(0), pTransaction->Fee);
			EXPECT_EQ(Timestamp(0), pTransaction->Deadline);

			validateTransaction(*pTransaction);
		}

		void AssertMosaicDefinitionName(
				const model::MosaicDefinitionTransaction& transaction,
				NamespaceId parentId,
				const std::string& mosaicName) {
			// Assert:
			EXPECT_EQ(mosaicName.size(), transaction.MosaicNameSize);
			EXPECT_EQ(0, memcmp(mosaicName.data(), transaction.NamePtr(), mosaicName.size()));

			// - id matches
			auto expectedId = model::GenerateMosaicId(parentId, mosaicName);
			EXPECT_EQ(expectedId, transaction.MosaicId);
		}

		auto CreatePropertyChecker(
				NamespaceId namespaceId,
				const std::string& mosaicName,
				model::MosaicFlags flags,
				uint8_t divisibility) {
			return [namespaceId, &mosaicName, flags, divisibility](const auto& transaction) {
				EXPECT_EQ(flags, transaction.PropertiesHeader.Flags);
				EXPECT_EQ(divisibility, transaction.PropertiesHeader.Divisibility);
				AssertMosaicDefinitionName(transaction, namespaceId, mosaicName);
			};
		}

		auto CreatePropertyChecker(
				NamespaceId namespaceId,
				const std::string& mosaicName,
				model::MosaicFlags flags,
				uint8_t divisibility,
				std::initializer_list<uint64_t> optionalValues) {
			return [namespaceId, &mosaicName, flags, divisibility, optionalValues](const auto& transaction) {
				CreatePropertyChecker(namespaceId, mosaicName, flags, divisibility)(transaction);

				auto expectedId = model::First_Optional_Property;
				for (auto optionalValue : optionalValues) {
					auto pProperties = transaction.PropertiesPtr();
					EXPECT_EQ(pProperties[0].Id, static_cast<model::MosaicPropertyId>(expectedId));
					EXPECT_EQ(pProperties[0].Value, optionalValue);
					++expectedId;
				}
			};
		}
	}

	// region constructor

	TEST(TEST_CLASS, CanCreateTransaction) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction(
				namespaceId,
				mosaicName,
				0,
				[](const auto&) {},
				CreatePropertyChecker(namespaceId, mosaicName, model::MosaicFlags::None, 0));
	}

	// endregion

	// region name validation

	TEST(TEST_CLASS, CannotSetEmptyName) {
		// Act + Assert:
		EXPECT_THROW(CreateBuilderWithName({}), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannnotSetTooLongName) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(1 << (sizeof(model::MosaicDefinitionTransaction::MosaicNameSize) * 8));
		auto builder = CreateBuilderWithName(namespaceName);

		// Assert:
		EXPECT_THROW(builder.build(), catapult_runtime_error);
	}

	// endregion

	// region required properties

	namespace {
		void AssertCanSetFlags(
				model::MosaicFlags expectedFlags,
				const std::function<void (MosaicDefinitionBuilder&)>& buildTransaction) {
			// Arrange:
			auto namespaceId = test::GenerateRandomValue<NamespaceId>();
			auto mosaicName = test::GenerateRandomString(10);

			// Assert:
			AssertCanBuildTransaction(
					namespaceId,
					mosaicName,
					0,
					buildTransaction,
					CreatePropertyChecker(namespaceId, mosaicName, expectedFlags, 0));
		}
	}

	TEST(TEST_CLASS, CanSetFlags_SupplyMutable) {
		// Assert:
		AssertCanSetFlags(
				model::MosaicFlags::Supply_Mutable,
				[](auto& builder) {
					builder.setSupplyMutable();
				});
	}

	TEST(TEST_CLASS, CanSetFlags_Transferable) {
		// Assert:
		AssertCanSetFlags(
				model::MosaicFlags::Transferable,
				[](auto& builder) {
					builder.setTransferable();
				});
	}

	TEST(TEST_CLASS, CanSetFlags_LevyMutable) {
		// Assert:
		AssertCanSetFlags(
				model::MosaicFlags::Levy_Mutable,
				[](auto& builder) {
					builder.setLevyMutable();
				});
	}

	TEST(TEST_CLASS, CanSetFlags_All) {
		// Assert:
		AssertCanSetFlags(
				model::MosaicFlags::Supply_Mutable | model::MosaicFlags::Transferable | model::MosaicFlags::Levy_Mutable ,
				[](auto& builder) {
					builder.setSupplyMutable();
					builder.setLevyMutable();
					builder.setTransferable();
				});
	}

	TEST(TEST_CLASS, CanSetDivisibility) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction(
				namespaceId,
				mosaicName,
				0,
				[](auto& builder) {
					builder.setDivisibility(0xA5);
				},
				CreatePropertyChecker(namespaceId, mosaicName, model::MosaicFlags::None, 0xA5));
	}

	// endregion

	// region optional properties

	TEST(TEST_CLASS, CanSetOptionalProperty_Duration) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction(
				namespaceId,
				mosaicName,
				sizeof(model::MosaicProperty),
				[](auto& builder) {
					builder.setDuration(ArtifactDuration(12345678));
				},
				CreatePropertyChecker(namespaceId, mosaicName, model::MosaicFlags::None, 0, { 12345678 }));
	}

	// endregion
}}
