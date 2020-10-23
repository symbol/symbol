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

#include "src/builders/AccountKeyLinkBuilder.h"
#include "src/builders/NodeKeyLinkBuilder.h"
#include "src/builders/VotingKeyLinkBuilder.h"
#include "src/builders/VrfKeyLinkBuilder.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"

namespace catapult { namespace builders {

#define TEST_CLASS KeyLinkBuilderTests

	namespace {
		template<typename TTransaction>
		auto GetLinkedPublicKey(const TTransaction& transaction) {
			return transaction.LinkedPublicKey;
		}

		struct AccountKeyLinkTestTraits {
			using BuilderType = AccountKeyLinkBuilder;
			using LinkedType = Key;
			static constexpr auto Transaction_Type = model::Entity_Type_Account_Key_Link;

			static void SetKey(BuilderType& builder, const LinkedType& key) {
				builder.setLinkedPublicKey(key);
			}
		};

		struct AccountKeyLinkTransactionTraits {
			using Regular = test::RegularTransactionTraits<model::AccountKeyLinkTransaction>;
			using Embedded = test::EmbeddedTransactionTraits<model::EmbeddedAccountKeyLinkTransaction>;
		};

		struct NodeKeyLinkTestTraits {
			using BuilderType = NodeKeyLinkBuilder;
			using LinkedType = Key;
			static constexpr auto Transaction_Type = model::Entity_Type_Node_Key_Link;

			static void SetKey(BuilderType& builder, const LinkedType& key) {
				builder.setLinkedPublicKey(key);
			}
		};

		struct NodeKeyLinkTransactionTraits {
			using Regular = test::RegularTransactionTraits<model::NodeKeyLinkTransaction>;
			using Embedded = test::EmbeddedTransactionTraits<model::EmbeddedNodeKeyLinkTransaction>;
		};

		struct VotingKeyLinkTestTraits {
			using BuilderType = VotingKeyLinkBuilder;
			using LinkedType = VotingKey;
			static constexpr auto Transaction_Type = model::Entity_Type_Voting_Key_Link;

			static void SetKey(BuilderType& builder, const LinkedType& key) {
				builder.setLinkedPublicKey(key);
			}
		};

		struct VotingKeyLinkTransactionTraits {
			using Regular = test::RegularTransactionTraits<model::VotingKeyLinkTransaction>;
			using Embedded = test::EmbeddedTransactionTraits<model::EmbeddedVotingKeyLinkTransaction>;
		};

		struct VrfKeyLinkTestTraits {
			using BuilderType = VrfKeyLinkBuilder;
			using LinkedType = Key;
			static constexpr auto Transaction_Type = model::Entity_Type_Vrf_Key_Link;

			static void SetKey(BuilderType& builder, const LinkedType& key) {
				builder.setLinkedPublicKey(key);
			}
		};

		struct VrfKeyLinkTransactionTraits {
			using Regular = test::RegularTransactionTraits<model::VrfKeyLinkTransaction>;
			using Embedded = test::EmbeddedTransactionTraits<model::EmbeddedVrfKeyLinkTransaction>;
		};

		template<typename TExpectedTraits, typename TTransactionTraits>
		struct KeyLinkTraits : public TExpectedTraits {
		public:
			static constexpr uint8_t Expected_Version = 1;

			using Builder = typename TExpectedTraits::BuilderType;
			using TransactionTraits = TTransactionTraits;

		public:
			struct TransactionProperties {
			public:
				explicit TransactionProperties(model::LinkAction linkAction)
						: LinkAction(linkAction)
						, LinkedPublicKey()
				{}

			public:
				model::LinkAction LinkAction;
				typename TExpectedTraits::LinkedType LinkedPublicKey;
			};

		public:
			template<typename TTransaction>
			static void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
				EXPECT_EQ(expectedProperties.LinkAction, transaction.LinkAction);
				EXPECT_EQ(expectedProperties.LinkedPublicKey, GetLinkedPublicKey(transaction));
			}
		};

		template<typename TTransactionTraits>
		struct VotingKeyLinkTraits : public VotingKeyLinkTestTraits {
		public:
			static constexpr uint8_t Expected_Version = 2;

			using Builder = typename VotingKeyLinkTestTraits::BuilderType;
			using TransactionTraits = TTransactionTraits;
			using AssertTraits = KeyLinkTraits<VotingKeyLinkTestTraits, TTransactionTraits>;

		public:
			struct TransactionProperties : public AssertTraits::TransactionProperties {
			public:
				explicit TransactionProperties(model::LinkAction linkAction)
						: AssertTraits::TransactionProperties(linkAction)
						, StartEpoch()
						, EndEpoch()
				{}

			public:
				FinalizationEpoch StartEpoch;
				FinalizationEpoch EndEpoch;
			};

		public:
			template<typename TTransaction>
			static void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
				AssertTraits::AssertTransactionProperties(expectedProperties, transaction);
				EXPECT_EQ(expectedProperties.StartEpoch, transaction.StartEpoch);
				EXPECT_EQ(expectedProperties.EndEpoch, transaction.EndEpoch);
			}
		};

		using AccountKeyLinkRegularTraits = KeyLinkTraits<AccountKeyLinkTestTraits, AccountKeyLinkTransactionTraits::Regular>;
		using AccountKeyLinkEmbeddedTraits = KeyLinkTraits<AccountKeyLinkTestTraits, AccountKeyLinkTransactionTraits::Embedded>;
		using NodeKeyLinkRegularTraits = KeyLinkTraits<NodeKeyLinkTestTraits, NodeKeyLinkTransactionTraits::Regular>;
		using NodeKeyLinkEmbeddedTraits = KeyLinkTraits<NodeKeyLinkTestTraits, NodeKeyLinkTransactionTraits::Embedded>;
		using VrfKeyLinkRegularTraits = KeyLinkTraits<VrfKeyLinkTestTraits, VrfKeyLinkTransactionTraits::Regular>;
		using VrfKeyLinkEmbeddedTraits = KeyLinkTraits<VrfKeyLinkTestTraits, VrfKeyLinkTransactionTraits::Embedded>;

		using VotingKeyLinkRegularTraits = VotingKeyLinkTraits<VotingKeyLinkTransactionTraits::Regular>;
		using VotingKeyLinkEmbeddedTraits = VotingKeyLinkTraits<VotingKeyLinkTransactionTraits::Embedded>;

		template<typename TTraits>
		void AssertCanBuildTransaction(
				const typename TTraits::TransactionProperties& expectedProperties,
				const consumer<typename TTraits::Builder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			typename TTraits::Builder builder(networkIdentifier, signer);
			buildTransaction(builder);
			auto pTransaction = TTraits::TransactionTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::TransactionTraits::CheckBuilderSize(0, builder);
			TTraits::TransactionTraits::CheckFields(0, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(TTraits::Expected_Version, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(TTraits::Transaction_Type, pTransaction->Type);

			TTraits::AssertTransactionProperties(expectedProperties, *pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Account_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountKeyLinkRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Account_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AccountKeyLinkEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Node_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NodeKeyLinkRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Node_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NodeKeyLinkEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Vrf_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VrfKeyLinkRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Vrf_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VrfKeyLinkEmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Voting_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VotingKeyLinkRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Voting_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VotingKeyLinkEmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region constructor

	TRAITS_BASED_TEST(CanCreateTransaction) {
		// Arrange:
		auto expectedProperties = typename TTraits::TransactionProperties(model::LinkAction::Unlink);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](const auto&) {});
	}

	// endregion

	// region settings

	TRAITS_BASED_TEST(CanSetLinkedPublicKey) {
		// Arrange:
		auto expectedProperties = typename TTraits::TransactionProperties(model::LinkAction::Unlink);
		test::FillWithRandomData(expectedProperties.LinkedPublicKey);
		const auto& linkedPublicKey = expectedProperties.LinkedPublicKey;

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [&linkedPublicKey](auto& builder) {
			TTraits::SetKey(builder, linkedPublicKey);
		});
	}

	TRAITS_BASED_TEST(CanSetAction) {
		// Arrange:
		auto expectedProperties = typename TTraits::TransactionProperties(static_cast<model::LinkAction>(0x45));

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setLinkAction(static_cast<model::LinkAction>(0x45));
		});
	}

	TRAITS_BASED_TEST(CanSetLinkedPublicKeyAndAction) {
		// Arrange:
		auto linkAction = static_cast<model::LinkAction>(0x45);
		auto expectedProperties = typename TTraits::TransactionProperties(linkAction);
		test::FillWithRandomData(expectedProperties.LinkedPublicKey);
		const auto& linkedPublicKey = expectedProperties.LinkedPublicKey;

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [linkAction, &linkedPublicKey](auto& builder) {
			TTraits::SetKey(builder, linkedPublicKey);
			builder.setLinkAction(linkAction);
		});
	}

	// endregion

	// region voting key link builder tests

#define VOTING_KEY_LINK_BUILDER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Voting_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VotingKeyLinkRegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Voting_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VotingKeyLinkEmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	VOTING_KEY_LINK_BUILDER_TEST(CanSetStartEpoch) {
		// Arrange:
		auto expectedProperties = typename TTraits::TransactionProperties(model::LinkAction::Unlink);
		expectedProperties.StartEpoch = FinalizationEpoch(0x12345);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setStartEpoch(FinalizationEpoch(0x12345));
		});
	}

	VOTING_KEY_LINK_BUILDER_TEST(CanSetEndEpoch) {
		// Arrange:
		auto expectedProperties = typename TTraits::TransactionProperties(model::LinkAction::Unlink);
		expectedProperties.EndEpoch = FinalizationEpoch(0x54321);

		// Assert:
		AssertCanBuildTransaction<TTraits>(expectedProperties, [](auto& builder) {
			builder.setEndEpoch(FinalizationEpoch(0x54321));
		});
	}

	// endregion
}}
