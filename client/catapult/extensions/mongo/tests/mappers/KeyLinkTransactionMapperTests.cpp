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

#include "mongo/src/mappers/KeyLinkTransactionMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/coresystem/src/model/VotingKeyLinkTransaction.h"
#include "plugins/coresystem/src/model/VrfKeyLinkTransaction.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTransactionPluginTests.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS KeyLinkTransactionMapperTests

	namespace {
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(VotingKeyLink, Voting)
		DEFINE_MONGO_TRANSACTION_PLUGIN_TEST_TRAITS_NO_ADAPT(VrfKeyLink, Vrf)
	}

	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Voting, _Voting, model::Entity_Type_Voting_Key_Link)
	DEFINE_BASIC_MONGO_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Vrf, _Vrf, model::Entity_Type_Vrf_Key_Link)

#undef PLUGIN_TEST

#define PLUGIN_TEST_ENTRY(NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_##NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NAME##RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_##NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NAME##EmbeddedTraits>(); } \

#define PLUGIN_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	PLUGIN_TEST_ENTRY(Voting, TEST_NAME) \
	PLUGIN_TEST_ENTRY(Vrf, TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region streamTransaction

	PLUGIN_TEST(CanMapLinkTransaction) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&transaction), sizeof(typename TTraits::TransactionType) });
		transaction.LinkAction = model::LinkAction::Unlink;
		transaction.Version = 2;

		auto pPlugin = TTraits::CreatePlugin();

		// Act:
		mappers::bson_stream::document builder;
		pPlugin->streamTransaction(builder, transaction);
		auto view = builder.view();

		// Assert:
		if constexpr (std::is_same_v<VotingKey, decltype(transaction.LinkedPublicKey)>) {
			EXPECT_EQ(4u, test::GetFieldCount(view));
			EXPECT_EQ(model::LinkAction::Unlink, static_cast<model::LinkAction>(test::GetUint32(view, "linkAction")));
			EXPECT_EQ(transaction.LinkedPublicKey, test::GetVotingKeyValue(view, "linkedPublicKey"));
			EXPECT_EQ(transaction.StartEpoch, FinalizationEpoch(test::GetUint32(view, "startEpoch")));
			EXPECT_EQ(transaction.EndEpoch, FinalizationEpoch(test::GetUint32(view, "endEpoch")));
		} else {
			EXPECT_EQ(2u, test::GetFieldCount(view));
			EXPECT_EQ(model::LinkAction::Unlink, static_cast<model::LinkAction>(test::GetUint32(view, "linkAction")));
			EXPECT_EQ(transaction.LinkedPublicKey, test::GetKeyValue(view, "linkedPublicKey"));
		}
	}

	namespace {
		template<typename TTransaction, typename TTraits>
		void AssertCanMapLinkTransaction_VotingV1() {
			// Arrange:
			TTransaction transaction;
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&transaction), sizeof(TTransaction) });
			transaction.LinkAction = model::LinkAction::Unlink;
			transaction.Version = 1;

			auto pPlugin = TTraits::CreatePlugin();

			// Act:
			mappers::bson_stream::document builder;
			pPlugin->streamTransaction(builder, transaction);
			auto view = builder.view();

			// Assert:
			EXPECT_EQ(4u, test::GetFieldCount(view));
			EXPECT_EQ(model::LinkAction::Unlink, static_cast<model::LinkAction>(test::GetUint32(view, "linkAction")));
			EXPECT_EQ(transaction.LinkedPublicKey, test::GetVotingKeyValue(view, "linkedPublicKey"));
			EXPECT_EQ(transaction.StartEpoch, FinalizationEpoch(test::GetUint32(view, "startEpoch")));
			EXPECT_EQ(transaction.EndEpoch, FinalizationEpoch(test::GetUint32(view, "endEpoch")));

			// - check key padding
			auto fullLinkedPublicKey = view["linkedPublicKey"].get_binary();
			EXPECT_EQ(48u, fullLinkedPublicKey.size);
			EXPECT_EQ_MEMORY((std::array<uint8_t, 16>().data()), fullLinkedPublicKey.bytes + VotingKey::Size, 16);
		}
	}

	TEST(TEST_CLASS, CanMapLinkTransaction_VotingV1_Regular) {
		AssertCanMapLinkTransaction_VotingV1<model::VotingKeyLinkV1Transaction, VotingRegularTraits>();
	}

	TEST(TEST_CLASS, CanMapLinkTransaction_VotingV1_Embedded) {
		AssertCanMapLinkTransaction_VotingV1<model::EmbeddedVotingKeyLinkV1Transaction, VotingEmbeddedTraits>();
	}

	// endregion
}}}
