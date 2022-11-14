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

#include "src/builders/NamespaceRegistrationBuilder.h"
#include "plugins/txes/namespace/src/model/NamespaceIdGenerator.h"
#include "catapult/constants.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS NamespaceRegistrationBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::NamespaceRegistrationTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedNamespaceRegistrationTransaction>;

		using TransactionType = model::NamespaceRegistrationTransaction;
		using TransactionPtr = std::unique_ptr<TransactionType>;

		struct TransactionProperties {
		public:
			explicit TransactionProperties(BlockDuration duration)
					: RegistrationType(model::NamespaceRegistrationType::Root)
					, Duration(duration)
					, NamespaceName(test::GenerateRandomString(10))

			{}

			explicit TransactionProperties(NamespaceId parentId)
					: RegistrationType(model::NamespaceRegistrationType::Child)
					, ParentId(parentId)
					, NamespaceName(test::GenerateRandomString(10))
			{}

		public:
			model::NamespaceRegistrationType RegistrationType;
			NamespaceId ParentId;
			BlockDuration Duration;
			std::string NamespaceName;
		};

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.RegistrationType, transaction.RegistrationType);
			if (model::NamespaceRegistrationType::Root == expectedProperties.RegistrationType)
				EXPECT_EQ(expectedProperties.Duration, transaction.Duration);
			else
				EXPECT_EQ(expectedProperties.ParentId, transaction.ParentId);

			// - name matches
			ASSERT_EQ(expectedProperties.NamespaceName.size(), transaction.NameSize);
			EXPECT_EQ_MEMORY(expectedProperties.NamespaceName.data(), transaction.NamePtr(), expectedProperties.NamespaceName.size());

			// - id matches
			auto expectedId = model::GenerateNamespaceId(expectedProperties.ParentId, expectedProperties.NamespaceName);
			EXPECT_EQ(expectedId, transaction.Id);
		}

		template<typename TTraits>
		void AssertCanBuildTransaction(
				const TransactionProperties& expectedProperties,
				const consumer<NamespaceRegistrationBuilder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();
			const auto& namespaceName = expectedProperties.NamespaceName;

			// Act:
			auto builder = NamespaceRegistrationBuilder(networkIdentifier, signer);
			builder.setName({ reinterpret_cast<const uint8_t*>(namespaceName.data()), namespaceName.size() });
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(namespaceName.size(), builder);
			TTraits::CheckFields(namespaceName.size(), *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(model::Entity_Type_Namespace_Registration, pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
		}

		void RunBuilderTest(const consumer<NamespaceRegistrationBuilder&>& buildTransaction) {
			// Arrange:
			NamespaceRegistrationBuilder builder(static_cast<model::NetworkIdentifier>(0x62), test::GenerateRandomByteArray<Key>());

			// Act:
			buildTransaction(builder);
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
		auto expectedProperties = TransactionProperties(BlockDuration(0));

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](const auto&) {});
	}

	// endregion

	// region name validation

	TEST(TEST_CLASS, CannotSetEmptyName) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			auto name = std::string();

			// Act + Assert:
			EXPECT_THROW(builder.setName({ reinterpret_cast<const uint8_t*>(name.data()), name.size() }), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, CannotChangeName) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			auto name = std::string("abc");
			builder.setName({ reinterpret_cast<const uint8_t*>(name.data()), name.size() });

			// Act + Assert:
			EXPECT_THROW(builder.setName({ reinterpret_cast<const uint8_t*>(name.data()), name.size() }), catapult_runtime_error);
		});
	}

	// endregion

	// region other properties

	TRAITS_BASED_TEST(CanSetDuration) {
		// Arrange:
		auto expectedProperties = TransactionProperties(BlockDuration(1234));

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setDuration(BlockDuration(1234));
		});
	}

	TRAITS_BASED_TEST(CanSetParentId) {
		// Arrange:
		auto expectedProperties = TransactionProperties(NamespaceId(1234));

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setParentId(NamespaceId(1234));
		});
	}

	TRAITS_BASED_TEST(LastOfParentIdAndDurationTakesPrecedence) {
		// Arrange:
		auto expectedProperties = TransactionProperties(NamespaceId(1234));

		// Assert: since the parent id is set last, the duration is ignored
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setDuration(BlockDuration(9876));
			builder.setParentId(NamespaceId(1234));
		});
	}

	// endregion
}}
