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

#include "catapult/ionet/BannedNodes.h"
#include "catapult/config/NodeConfiguration.h"
#include "tests/test/nodeps/TimeSupplier.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS BannedNodesTests

	// region utils

	namespace {
		constexpr auto Failure_Reason = 0x80000000;
		constexpr auto Default_Strategy = model::NodeIdentityEqualityStrategy::Key;

		model::NodeIdentity CreateRandomIdentity() {
			return model::NodeIdentity{ test::GenerateRandomByteArray<Key>(), "11.22.33.44" };
		}

		std::vector<model::NodeIdentity> CreateRandomIdentities(size_t count) {
			std::vector<model::NodeIdentity> identities;
			for (auto i = 0u; i < count; ++i)
				identities.push_back(CreateRandomIdentity());

			return identities;
		}

		BanSettings CreateBanSettings() {
			BanSettings settings;
			settings.DefaultBanDuration = utils::TimeSpan::FromHours(2);
			settings.MaxBanDuration = utils::TimeSpan::FromHours(5);
			settings.KeepAliveDuration = utils::TimeSpan::FromHours(4);
			settings.MaxBannedNodes = 5;
			return settings;
		}

		supplier<Timestamp> CreateTimeSupplier(const std::vector<uint32_t>& rawTimestamps) {
			return test::CreateTimeSupplierFromMilliseconds(rawTimestamps, 1000 * 60 * 60);
		}

		void AssertCanBanNodes(size_t count) {
			// Arrange:
			BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1 }), Default_Strategy);
			std::vector<model::NodeIdentity> bannedNodeIdentities;

			// Act:
			for (auto i = 0u; i < count; ++i) {
				bannedNodeIdentities.push_back(CreateRandomIdentity());
				bannedNodes.add(bannedNodeIdentities.back(), Failure_Reason);
			}

			// Assert:
			EXPECT_EQ(count, bannedNodes.size());
			EXPECT_EQ(count, bannedNodes.deepSize());

			for (const auto& identity : bannedNodeIdentities)
				EXPECT_TRUE(bannedNodes.isBanned(identity)) << identity;
		}
	}

	// endregion

	// region basic

	TEST(TEST_CLASS, InitiallyNoNodeIsBanned) {
		// Arrange:
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1 }), Default_Strategy);

		// Act + Assert:
		EXPECT_EQ(0u, bannedNodes.size());
		EXPECT_EQ(0u, bannedNodes.deepSize());
	}

	TEST(TEST_CLASS, CanBanNodes) {
		AssertCanBanNodes(1);
		AssertCanBanNodes(2);
		AssertCanBanNodes(5);
	}

	// endregion

	// region isBanned

	TEST(TEST_CLASS, IsBannedReturnsTrueForBannedNodes) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1 }), Default_Strategy);
		bannedNodes.add(identity, Failure_Reason);

		// Sanity:
		EXPECT_EQ(1u, bannedNodes.size());
		EXPECT_EQ(1u, bannedNodes.deepSize());

		// Act:
		auto result = bannedNodes.isBanned(identity);

		// Assert:
		EXPECT_TRUE(result);
	}

	TEST(TEST_CLASS, IsBannedReturnsFalseForUnbannedNodes) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1 }), Default_Strategy);
		bannedNodes.add(identity, Failure_Reason);

		// Sanity:
		EXPECT_EQ(1u, bannedNodes.size());
		EXPECT_EQ(1u, bannedNodes.deepSize());

		// Act:
		auto result = bannedNodes.isBanned(CreateRandomIdentity());

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, IsBannedReturnsFalseForBannedNodesWhereBanHasExpired) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1, 3, 4, 19, 26 }), Default_Strategy);
		bannedNodes.add(identity, Failure_Reason); // 1h

		// Sanity:
		EXPECT_EQ(0u, bannedNodes.size()); // 3h
		EXPECT_EQ(1u, bannedNodes.deepSize());

		// Act + Assert: timestamps are 4h, 19h, 26h
		for (auto i = 0u; i < 3; ++i)
			EXPECT_FALSE(bannedNodes.isBanned(identity)) << "at index " << i;
	}

	// endregion

	// region add

	TEST(TEST_CLASS, BanExpiresAfterBanDuration) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1, 1, 2, 3 }), Default_Strategy);

		// Act:
		bannedNodes.add(identity, Failure_Reason);

		// Assert: ban ends at 3h
		EXPECT_TRUE(bannedNodes.isBanned(identity)); // 1h
		EXPECT_TRUE(bannedNodes.isBanned(identity)); // 2h
		EXPECT_FALSE(bannedNodes.isBanned(identity)); // 3h
	}

	TEST(TEST_CLASS, SuccessiveBanIncreasesBanTimeSpan) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1, 1, 3, 4, 5 }), Default_Strategy);

		// Act: 2 + 2 hours
		bannedNodes.add(identity, Failure_Reason);
		bannedNodes.add(identity, Failure_Reason);

		// Assert: ban ends at 5h
		EXPECT_TRUE(bannedNodes.isBanned(identity)); // 3h
		EXPECT_TRUE(bannedNodes.isBanned(identity)); // 4h
		EXPECT_FALSE(bannedNodes.isBanned(identity)); // 5h
	}

	TEST(TEST_CLASS, SuccessiveBansRespectMaxBanTimeSpan) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1, 1, 1, 4, 5, 6 }), Default_Strategy);

		// Act: 2 + 2 + 2 hours > max ban time span == 5 hours
		bannedNodes.add(identity, Failure_Reason);
		bannedNodes.add(identity, Failure_Reason);
		bannedNodes.add(identity, Failure_Reason);

		// Assert: ban ends at 6h
		EXPECT_TRUE(bannedNodes.isBanned(identity)); // 4h
		EXPECT_TRUE(bannedNodes.isBanned(identity)); // 5h
		EXPECT_FALSE(bannedNodes.isBanned(identity)); // 6h
	}

	TEST(TEST_CLASS, SuccessiveBansRespectMaxBannedNodes) {
		// Arrange:
		std::vector<model::NodeIdentity> identities;
		auto banSettings = CreateBanSettings();
		banSettings.DefaultBanDuration = utils::TimeSpan::FromHours(10);
		BannedNodes bannedNodes(banSettings, CreateTimeSupplier({ 4, 2, 1, 5, 3, 6, 7 }), Default_Strategy);
		for (auto i = 0u; i < banSettings.MaxBannedNodes; ++i) {
			identities.push_back(CreateRandomIdentity());
			bannedNodes.add(identities.back(), Failure_Reason);
		}

		// Sanity:
		EXPECT_EQ(banSettings.MaxBannedNodes, bannedNodes.deepSize());

		// Act:
		identities.push_back(CreateRandomIdentity());
		bannedNodes.add(identities.back(), Failure_Reason);

		// Assert: ban with ban start at 1h (index 2) expires first and therefore was removed
		for (const auto& identity : identities) {
			if (!model::NodeIdentityEquality(Default_Strategy)(identity, identities[2]))
				EXPECT_TRUE(bannedNodes.isBanned(identity)) << identity;
			else
				EXPECT_FALSE(bannedNodes.isBanned(identity)) << identity;
		}
	}

	TEST(TEST_CLASS, CannotBanNodeWithLocalNetworkHost) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		identity.Host = "_local_";
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1, 1, 1, 4, 5, 6 }), Default_Strategy);

		// Act:
		bannedNodes.add(identity, Failure_Reason);

		// Assert: identity was not banned
		EXPECT_EQ(0u, bannedNodes.size());
		EXPECT_EQ(0u, bannedNodes.deepSize());
		EXPECT_FALSE(bannedNodes.isBanned(identity));
	}

	// endregion

	// region prune

	TEST(TEST_CLASS, PruneDoesNotRemoveBannedNodeThatNeedsToBeKeptAlive) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1, 4, 5 }), Default_Strategy);
		bannedNodes.add(identity, Failure_Reason);

		// Act:
		bannedNodes.prune(); // 4h
		bannedNodes.prune(); // 5h

		// Assert: banned node is kept alive until (1 + 2 + 4)h
		EXPECT_EQ(0u, bannedNodes.size());
		EXPECT_EQ(1u, bannedNodes.deepSize());
	}

	TEST(TEST_CLASS, PruneRemovesBannedNodeAfterKeepAlivePeriodEnds) {
		// Arrange:
		auto identity = CreateRandomIdentity();
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1, 7 }), Default_Strategy);
		bannedNodes.add(identity, Failure_Reason);

		// Act: keep alive period ends at 7h
		bannedNodes.prune(); // 7h

		// Assert:
		EXPECT_EQ(0u, bannedNodes.size());
		EXPECT_EQ(0u, bannedNodes.deepSize());
	}

	TEST(TEST_CLASS, PruneRemovesAllBannedNodesWhereKeepAlivePeriodEnded) {
		// Arrange:
		auto identities = CreateRandomIdentities(6);
		BanSettings banSettings;
		banSettings.DefaultBanDuration = utils::TimeSpan::FromHours(1);
		banSettings.MaxBanDuration = utils::TimeSpan::FromHours(5);
		banSettings.KeepAliveDuration = utils::TimeSpan::FromHours(2);
		banSettings.MaxBannedNodes = 10;
		std::vector<uint32_t> rawTimestamps(6 * 7 / 2, 1);
		rawTimestamps.push_back(5);

		BannedNodes bannedNodes(banSettings, CreateTimeSupplier(rawTimestamps), Default_Strategy);
		// bans expire at 2h, 3h, 4h, 5h, 6h, 6h
		for (auto i = 0u; i < 6; ++i) {
			for (auto j = 0u; j <= i; ++j)
				bannedNodes.add(identities[i], Failure_Reason);
		}

		// Act: keep alive period end at 4h, 5h, 6h, 7h, 8h, 8h
		bannedNodes.prune(); // 5h

		// Assert: { 0, 1 } => pruned, { 2, 3 } => keep alive period, { 4, 5 } => banned
		EXPECT_EQ(2u, bannedNodes.size());
		EXPECT_EQ(4u, bannedNodes.deepSize());
		EXPECT_FALSE(bannedNodes.isBanned(identities[0]));
		EXPECT_FALSE(bannedNodes.isBanned(identities[1]));
		EXPECT_FALSE(bannedNodes.isBanned(identities[2]));
		EXPECT_FALSE(bannedNodes.isBanned(identities[3]));
		EXPECT_TRUE(bannedNodes.isBanned(identities[4]));
		EXPECT_TRUE(bannedNodes.isBanned(identities[5]));
	}

	// endregion

	// region node equality strategy

	TEST(TEST_CLASS, BannedNodesRespectsEqualityStrategy) {
		// Arrange: identity1 and identity2 are equal
		auto identity1 = CreateRandomIdentity();
		auto identity2 = CreateRandomIdentity();
		auto identity3 = CreateRandomIdentity();
		identity3.Host = "123.321.234.432";
		auto strategy = model::NodeIdentityEqualityStrategy::Host;
		BannedNodes bannedNodes(CreateBanSettings(), CreateTimeSupplier({ 1 }), strategy);
		bannedNodes.add(identity1, Failure_Reason);
		bannedNodes.add(identity2, Failure_Reason);
		bannedNodes.add(identity3, Failure_Reason);

		// Assert:
		EXPECT_TRUE(bannedNodes.isBanned(identity1));
		EXPECT_TRUE(bannedNodes.isBanned(identity2));
		EXPECT_TRUE(bannedNodes.isBanned(identity3));
		EXPECT_EQ(2u, bannedNodes.size());
		EXPECT_EQ(2u, bannedNodes.deepSize());
	}

	// endregion
}}
