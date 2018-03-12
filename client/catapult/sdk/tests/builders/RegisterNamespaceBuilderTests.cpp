#include "src/builders/RegisterNamespaceBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS RegisterNamespaceBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::RegisterNamespaceTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedRegisterNamespaceTransaction>;

		auto CreateBuilderWithName(const RawString& name, const Key& signer) {
			return RegisterNamespaceBuilder(static_cast<model::NetworkIdentifier>(0x62), signer, name);
		}

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransaction(
				const std::string& namespaceName,
				const consumer<RegisterNamespaceBuilder&>& buildTransaction,
				const TValidationFunction& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			RegisterNamespaceBuilder builder(networkId, signer, namespaceName);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(namespaceName.size(), *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Register_Namespace, pTransaction->Type);

			validateTransaction(*pTransaction);
		}

		template<typename TTransaction>
		void AssertNamespace(const TTransaction& transaction, NamespaceId parentId, const std::string& namespaceName) {
			// Assert:
			EXPECT_EQ(namespaceName.size(), transaction.NamespaceNameSize);
			EXPECT_TRUE(0 == std::memcmp(namespaceName.data(), transaction.NamePtr(), namespaceName.size()));

			// - id matches
			auto expectedId = model::GenerateNamespaceId(parentId, namespaceName);
			EXPECT_EQ(expectedId, transaction.NamespaceId);
		}

		auto CreateRootPropertyChecker(const std::string& namespaceName, BlockDuration namespaceDuration) {
			return [&namespaceName, namespaceDuration](const auto& transaction) {
				// Assert:
				EXPECT_EQ(model::NamespaceType::Root, transaction.NamespaceType);
				EXPECT_EQ(namespaceDuration, transaction.Duration);
				AssertNamespace(transaction, Namespace_Base_Id, namespaceName);
			};
		}

		auto CreateChildPropertyChecker(const std::string& namespaceName, NamespaceId parentId) {
			return [&namespaceName, parentId](const auto& transaction) {
				// Assert:
				EXPECT_EQ(model::NamespaceType::Child, transaction.NamespaceType);
				EXPECT_EQ(parentId, transaction.ParentId);
				AssertNamespace(transaction, parentId, namespaceName);
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
		auto namespaceName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				namespaceName,
				[](const auto&) {},
				CreateRootPropertyChecker(namespaceName, Eternal_Artifact_Duration));
	}

	// endregion

	// region name validation

	TEST(TEST_CLASS, CannotSetEmptyName) {
		// Act + Assert:
		EXPECT_THROW(CreateBuilderWithName({}, test::GenerateRandomData<Key_Size>()), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannnotSetTooLongName) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(1 << (sizeof(model::RegisterNamespaceTransaction::NamespaceNameSize) * 8));
		auto signer = test::GenerateRandomData<Key_Size>();
		auto builder = CreateBuilderWithName(namespaceName, signer);

		// Act + Assert:
		EXPECT_THROW(builder.build(), catapult_runtime_error);
	}

	// endregion

	// region other properties

	TRAITS_BASED_TEST(CanSetDuration) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				namespaceName,
				[](auto& builder) {
					builder.setDuration(BlockDuration(12345));
				},
				CreateRootPropertyChecker(namespaceName, BlockDuration(12345)));
	}

	TRAITS_BASED_TEST(CanSetParentId) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction<TTraits>(
				namespaceName,
				[](auto& builder) {
					builder.setParentId(NamespaceId(12345));
				},
				CreateChildPropertyChecker(namespaceName, NamespaceId(12345)));
	}

	TRAITS_BASED_TEST(ParentIdTakesPrecedenceOverDuration) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(10);

		// Assert: since the parent id is set, the duration is ignored
		AssertCanBuildTransaction<TTraits>(
				namespaceName,
				[](auto& builder) {
					builder.setParentId(NamespaceId(12345));
					builder.setDuration(BlockDuration(98765));
				},
				CreateChildPropertyChecker(namespaceName, NamespaceId(12345)));
	}

	// endregion
}}
