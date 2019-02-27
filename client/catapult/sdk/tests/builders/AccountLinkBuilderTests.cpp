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

#include "src/builders/AccountLinkBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS AccountLinkBuilderTests

	namespace {
		auto GetLinkedAccountKey(const model::AccountLinkTransaction& transaction) {
			return transaction.RemoteAccountKey;
		}

		auto GetLinkedAccountKey(const model::EmbeddedAccountLinkTransaction& transaction) {
			return transaction.RemoteAccountKey;
		}

		struct AccountLinkTestTraits {
			using LinkedType = Key;

			static constexpr auto Transaction_Type = model::Entity_Type_Account_Link;
		};

		template<typename TBuilder, typename TExpectedTraits>
		struct LinkTraits : public TExpectedTraits {
		public:
			using Builder = TBuilder;
			using RegularTraits = test::RegularTransactionTraits<typename Builder::Transaction>;
			using EmbeddedTraits = test::EmbeddedTransactionTraits<typename Builder::EmbeddedTransaction>;

		public:
			struct TransactionProperties {
			public:
				explicit TransactionProperties(model::AccountLinkAction linkAction)
						: LinkAction(linkAction)
						, LinkedAccountKey()
				{}

			public:
				model::AccountLinkAction LinkAction;
				typename TExpectedTraits::LinkedType LinkedAccountKey;
			};

		public:
			template<typename TTransaction>
			static void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
				EXPECT_EQ(expectedProperties.LinkAction, transaction.LinkAction);
				EXPECT_EQ(expectedProperties.LinkedAccountKey, GetLinkedAccountKey(transaction));
			}
		};

		using AccountLinkTraits = LinkTraits<AccountLinkBuilder, AccountLinkTestTraits>;

		template<typename TLinkTraits, typename TTraits>
		void AssertCanBuildTransaction(
				const typename TLinkTraits::TransactionProperties& expectedProperties,
				const consumer<typename TLinkTraits::Builder&>& buildTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();

			// Act:
			typename TLinkTraits::Builder builder(networkId, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(0, *pTransaction);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6202, pTransaction->Version);
			EXPECT_EQ(TLinkTraits::Transaction_Type, pTransaction->Type);

			TLinkTraits::AssertTransactionProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TLinkTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_RemoteKey_Regular) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountLinkTraits, AccountLinkTraits::RegularTraits>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_RemoteKey_Embedded) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountLinkTraits, AccountLinkTraits::EmbeddedTraits>(); \
	} \
	template<typename TLinkTraits, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransaction) {
		// Arrange:
		auto expectedProperties = typename TLinkTraits::TransactionProperties(model::AccountLinkAction::Link);

		// Assert:
		AssertCanBuildTransaction<TLinkTraits, TTraits>(expectedProperties, [](const auto&) {});
	}

	// endregion

	// region settings

	TRAITS_BASED_TEST(CanSetRemote) {
		// Arrange:
		auto expectedProperties = typename TLinkTraits::TransactionProperties(model::AccountLinkAction::Link);
		test::FillWithRandomData(expectedProperties.LinkedAccountKey);
		const auto& linkedAccountKey = expectedProperties.LinkedAccountKey;

		// Assert:
		AssertCanBuildTransaction<TLinkTraits, TTraits>(expectedProperties, [&linkedAccountKey](auto& builder) {
			builder.setRemoteAccountKey(linkedAccountKey);
		});
	}

	TRAITS_BASED_TEST(CanSetAction) {
		// Arrange:
		auto expectedProperties = typename TLinkTraits::TransactionProperties(static_cast<model::AccountLinkAction>(0x45));

		// Assert:
		AssertCanBuildTransaction<TLinkTraits, TTraits>(expectedProperties, [](auto& builder) {
			builder.setLinkAction(static_cast<model::AccountLinkAction>(0x45));
		});
	}

	TRAITS_BASED_TEST(CanSetRemoteAndAction) {
		// Arrange:
		auto linkAction = static_cast<model::AccountLinkAction>(0x45);
		auto expectedProperties = typename TLinkTraits::TransactionProperties(linkAction);
		test::FillWithRandomData(expectedProperties.LinkedAccountKey);
		const auto& linkedAccountKey = expectedProperties.LinkedAccountKey;

		// Assert:
		AssertCanBuildTransaction<TLinkTraits, TTraits>(expectedProperties, [linkAction, &linkedAccountKey](auto& builder) {
			builder.setRemoteAccountKey(linkedAccountKey);
			builder.setLinkAction(linkAction);
		});
	}

	// endregion
}}
