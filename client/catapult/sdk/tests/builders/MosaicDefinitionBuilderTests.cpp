#include "src/builders/MosaicDefinitionBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS MosaicDefinitionBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::MosaicDefinitionTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedMosaicDefinitionTransaction>;

		auto CreateBuilderWithName(const RawString& name, const Key& signer) {
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			return MosaicDefinitionBuilder(networkIdentifier, signer, test::GenerateRandomValue<NamespaceId>(), name);
		}

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransaction(
				NamespaceId parentNamespaceId,
				const std::string& mosaicName,
				size_t propertiesSize,
				const consumer<MosaicDefinitionBuilder&>& buildTransaction,
				const TValidationFunction& validateTransaction) {

			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			MosaicDefinitionBuilder builder(networkId, signer, parentNamespaceId, mosaicName);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(mosaicName.size() + propertiesSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Mosaic_Definition, pTransaction->Type);
			EXPECT_EQ(parentNamespaceId, pTransaction->ParentId);

			validateTransaction(*pTransaction);
		}

		template<typename TTransaction>
		void AssertMosaicDefinitionName(const TTransaction& transaction, NamespaceId parentId, const std::string& mosaicName) {
			// Assert:
			EXPECT_EQ(mosaicName.size(), transaction.MosaicNameSize);
			EXPECT_TRUE(0 == std::memcmp(mosaicName.data(), transaction.NamePtr(), mosaicName.size()));

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

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransaction) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
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
		EXPECT_THROW(CreateBuilderWithName({}, test::GenerateRandomData<Key_Size>()), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannnotSetTooLongName) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(1 << (sizeof(model::MosaicDefinitionTransaction::MosaicNameSize) * 8));
		auto signer = test::GenerateRandomData<Key_Size>();
		auto builder = CreateBuilderWithName(namespaceName, signer);

		// Act + Assert:
		EXPECT_THROW(builder.build(), catapult_runtime_error);
	}

	// endregion

	// region required properties

	namespace {
		template<typename TTraits>
		void AssertCanSetFlags(model::MosaicFlags expectedFlags, const consumer<MosaicDefinitionBuilder&>& buildTransaction) {
			// Arrange:
			auto namespaceId = test::GenerateRandomValue<NamespaceId>();
			auto mosaicName = test::GenerateRandomString(10);

			// Assert:
			AssertCanBuildTransaction<TTraits>(
					namespaceId,
					mosaicName,
					0,
					buildTransaction,
					CreatePropertyChecker(namespaceId, mosaicName, expectedFlags, 0));
		}
	}

	TRAITS_BASED_TEST(CanSetFlags_SupplyMutable) {
		// Assert:
		AssertCanSetFlags<TTraits>(
				model::MosaicFlags::Supply_Mutable,
				[](auto& builder) {
					builder.setSupplyMutable();
				});
	}

	TRAITS_BASED_TEST(CanSetFlags_Transferable) {
		// Assert:
		AssertCanSetFlags<TTraits>(
				model::MosaicFlags::Transferable,
				[](auto& builder) {
					builder.setTransferable();
				});
	}

	TRAITS_BASED_TEST(CanSetFlags_LevyMutable) {
		// Assert:
		AssertCanSetFlags<TTraits>(
				model::MosaicFlags::Levy_Mutable,
				[](auto& builder) {
					builder.setLevyMutable();
				});
	}

	TRAITS_BASED_TEST(CanSetFlags_All) {
		// Assert:
		AssertCanSetFlags<TTraits>(
				model::MosaicFlags::Supply_Mutable | model::MosaicFlags::Transferable | model::MosaicFlags::Levy_Mutable,
				[](auto& builder) {
					builder.setSupplyMutable();
					builder.setLevyMutable();
					builder.setTransferable();
				});
	}

	TRAITS_BASED_TEST(CanSetDivisibility) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
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

	TRAITS_BASED_TEST(CanSetOptionalProperty_Duration) {
		// Arrange:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		auto mosaicName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				namespaceId,
				mosaicName,
				sizeof(model::MosaicProperty),
				[](auto& builder) {
					builder.setDuration(BlockDuration(12345678));
				},
				CreatePropertyChecker(namespaceId, mosaicName, model::MosaicFlags::None, 0, { 12345678 }));
	}

	// endregion
}}
