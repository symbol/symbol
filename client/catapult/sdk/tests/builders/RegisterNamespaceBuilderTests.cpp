#include "src/builders/RegisterNamespaceBuilder.h"
#include "plugins/txes/namespace/src/model/IdGenerator.h"
#include "tests/TestHarness.h"

#define TEST_CLASS RegisterNamespaceBuilderTests

namespace catapult { namespace builders {

	namespace {
		auto CreateBuilderWithName(const RawString& name) {
			return RegisterNamespaceBuilder(
					static_cast<model::NetworkIdentifier>(0x62),
					test::GenerateRandomData<Key_Size>(),
					name);
		}

		void AssertCanBuildTransaction(
				const std::string& namespaceName,
				const std::function<void (RegisterNamespaceBuilder&)>& buildTransaction,
				const std::function<void (const model::RegisterNamespaceTransaction&)>& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			RegisterNamespaceBuilder builder(networkId, signer, namespaceName);
			buildTransaction(builder);
			auto pTransaction = builder.build();

			// Assert:
			ASSERT_EQ(sizeof(model::RegisterNamespaceTransaction) + namespaceName.size(), pTransaction->Size);
			EXPECT_EQ(Signature{}, pTransaction->Signature);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(model::EntityType::Register_Namespace, pTransaction->Type);

			EXPECT_EQ(Amount(0), pTransaction->Fee);
			EXPECT_EQ(Timestamp(0), pTransaction->Deadline);

			validateTransaction(*pTransaction);
		}

		void AssertNamespace(
				const model::RegisterNamespaceTransaction& transaction,
				NamespaceId parentId,
				const std::string& namespaceName) {
			// Assert:
			EXPECT_EQ(namespaceName.size(), transaction.NamespaceNameSize);
			EXPECT_EQ(0, memcmp(namespaceName.data(), transaction.NamePtr(), namespaceName.size()));

			// - id matches
			auto expectedId = model::GenerateNamespaceId(parentId, namespaceName);
			EXPECT_EQ(expectedId, transaction.NamespaceId);
		}

		auto CreateRootPropertyChecker(const std::string& namespaceName, ArtifactDuration namespaceDuration) {
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

	// region constructor

	TEST(TEST_CLASS, CanCreateTransaction) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction(
				namespaceName,
				[](const auto&) {},
				CreateRootPropertyChecker(namespaceName, Eternal_Artifact_Duration));
	}

	// endregion

	// region name validation

	TEST(TEST_CLASS, CannotSetEmptyName) {
		// Act + Assert:
		EXPECT_THROW(CreateBuilderWithName({}), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannnotSetTooLongName) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(1 << (sizeof(model::RegisterNamespaceTransaction::NamespaceNameSize) * 8));
		auto builder = CreateBuilderWithName(namespaceName);

		// Assert:
		EXPECT_THROW(builder.build(), catapult_runtime_error);
	}

	// endregion

	// region other properties

	TEST(TEST_CLASS, CanSetDuration) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction(
				namespaceName,
				[](auto& builder) {
					builder.setDuration(ArtifactDuration(12345));
				},
				CreateRootPropertyChecker(namespaceName, ArtifactDuration(12345)));
	}

	TEST(TEST_CLASS, CanSetParentId) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(10);

		// Assert:
		AssertCanBuildTransaction(
				namespaceName,
				[](auto& builder) {
					builder.setParentId(NamespaceId(12345));
				},
				CreateChildPropertyChecker(namespaceName, NamespaceId(12345)));
	}

	TEST(TEST_CLASS, ParentIdTakesPrecedenceOverDuration) {
		// Arrange:
		auto namespaceName = test::GenerateRandomString(10);

		// Assert: since the parent id is set, the duration is ignored
		AssertCanBuildTransaction(
				namespaceName,
				[](auto& builder) {
					builder.setParentId(NamespaceId(12345));
					builder.setDuration(ArtifactDuration(98765));
				},
				CreateChildPropertyChecker(namespaceName, NamespaceId(12345)));
	}

	// endregion
}}
