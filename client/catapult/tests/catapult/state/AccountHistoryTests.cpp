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

#include "catapult/state/AccountHistory.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountHistoryTests

	// region traits

	namespace {
		struct BalanceTraits {
			static Amount ToValue(uint8_t seed) {
				return Amount(seed);
			}

			static const auto& GetValueHistory(const AccountHistory& history) {
				return history.balance();
			}
		};

		struct VrfPublicKeyTraits {
			static Key ToValue(uint8_t seed) {
				return { { seed } };
			}

			static const auto& GetValueHistory(const AccountHistory& history) {
				return history.vrfPublicKey();
			}
		};

		struct VotingPublicKeysTraits {
			static std::vector<model::PinnedVotingKey> ToValue(uint8_t seed) {
				return {
					{ { { seed } }, FinalizationPoint(seed + 100), FinalizationPoint(seed + 150) },
					{ { { static_cast<uint8_t>(seed * seed) } }, FinalizationPoint(seed + 151), FinalizationPoint(seed + 175) }
				};
			}

			static const auto& GetValueHistory(const AccountHistory& history) {
				return history.votingPublicKeys();
			}
		};
	}

#define HISTORY_VALUE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Balance) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BalanceTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_VrfPublicKey) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VrfPublicKeyTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_VotingPublicKeys) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<VotingPublicKeysTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region constructor

	TEST(TEST_CLASS, CanCreateEmpty) {
		// Act:
		AccountHistory history;

		// Assert:
		EXPECT_EQ(0u, history.balance().size());
		EXPECT_EQ(0u, history.vrfPublicKey().size());
		EXPECT_EQ(0u, history.votingPublicKeys().size());
	}

	// endregion

	// region anyAtLeast

	TEST(TEST_CLASS, AnyAtLeastReturnsFalseWhenEmpty) {
		// Arrange:
		AccountHistory history;

		// Act + Assert:
		EXPECT_FALSE(history.anyAtLeast(Amount(0)));
	}

	TEST(TEST_CLASS, AnyAtLeastReturnsCorrectValueWhenNotEmpty) {
		// Arrange:
		AccountHistory history;
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(98));
		history.add(Height(33), Amount(67));

		// Act + Assert:
		EXPECT_TRUE(history.anyAtLeast(Amount(0)));
		EXPECT_TRUE(history.anyAtLeast(Amount(11)));
		EXPECT_TRUE(history.anyAtLeast(Amount(12)));
		EXPECT_TRUE(history.anyAtLeast(Amount(67)));
		EXPECT_TRUE(history.anyAtLeast(Amount(68)));
		EXPECT_TRUE(history.anyAtLeast(Amount(98)));

		EXPECT_FALSE(history.anyAtLeast(Amount(99)));
		EXPECT_FALSE(history.anyAtLeast(Amount(100)));
		EXPECT_FALSE(history.anyAtLeast(Amount(1000)));
		EXPECT_FALSE(history.anyAtLeast(Amount(10000)));
	}

	// endregion

	// region add

	HISTORY_VALUE_TEST(CanAddSingleValue) {
		// Arrange:
		AccountHistory history;
		const auto& valueHistory = TTraits::GetValueHistory(history);

		// Act:
		history.add(Height(11), TTraits::ToValue(22));

		// Assert:
		EXPECT_EQ(1u, valueHistory.size());
		EXPECT_EQ(std::vector<Height>({ Height(11) }), valueHistory.heights());
		EXPECT_EQ(TTraits::ToValue(22), valueHistory.get());

		EXPECT_EQ(TTraits::ToValue(22), valueHistory.get(Height(11)));
	}

	HISTORY_VALUE_TEST(CanAddMultipleHomogenousValues) {
		// Arrange:
		AccountHistory history;
		const auto& valueHistory = TTraits::GetValueHistory(history);

		// Act:
		history.add(Height(11), TTraits::ToValue(12));
		history.add(Height(22), TTraits::ToValue(98));
		history.add(Height(33), TTraits::ToValue(67));

		// Assert:
		EXPECT_EQ(3u, valueHistory.size());
		EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), valueHistory.heights());
		EXPECT_EQ(TTraits::ToValue(67), valueHistory.get());

		EXPECT_EQ(TTraits::ToValue(12), valueHistory.get(Height(11)));
		EXPECT_EQ(TTraits::ToValue(98), valueHistory.get(Height(22)));
		EXPECT_EQ(TTraits::ToValue(67), valueHistory.get(Height(33)));
	}

	TEST(TEST_CLASS, CanAddMultipleHeterogenousValues) {
		// Arrange:
		AccountHistory history;

		// Act:
		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(98));
		history.add(Height(33), Amount(67));

		history.add(Height(12), Key{ { 100 } });

		history.add(Height(22), VotingPublicKeysTraits::ToValue(75));
		history.add(Height(44), VotingPublicKeysTraits::ToValue(200));

		// Assert:
		{
			const auto& valueHistory = history.balance();
			EXPECT_EQ(3u, valueHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(11), Height(22), Height(33) }), valueHistory.heights());
			EXPECT_EQ(Amount(67), valueHistory.get());

			EXPECT_EQ(Amount(12), valueHistory.get(Height(11)));
			EXPECT_EQ(Amount(98), valueHistory.get(Height(22)));
			EXPECT_EQ(Amount(67), valueHistory.get(Height(33)));
		}

		{
			const auto& valueHistory = history.vrfPublicKey();
			EXPECT_EQ(1u, valueHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(12) }), valueHistory.heights());
			EXPECT_EQ(Key{ { 100 } }, valueHistory.get());

			EXPECT_EQ(Key{ { 100 } }, valueHistory.get(Height(12)));
		}

		{
			const auto& valueHistory = history.votingPublicKeys();
			EXPECT_EQ(2u, valueHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(22), Height(44) }), valueHistory.heights());
			EXPECT_EQ(VotingPublicKeysTraits::ToValue(200), valueHistory.get());

			EXPECT_EQ(VotingPublicKeysTraits::ToValue(75), valueHistory.get(Height(22)));
			EXPECT_EQ(VotingPublicKeysTraits::ToValue(200), valueHistory.get(Height(44)));
		}
	}

	// endregion

	// region prune

	namespace {
		template<typename TTraits>
		void RunPruneTest(Height pruneHeight, const consumer<const AccountHistory&>& checkHistory) {
			// Arrange:
			AccountHistory history;
			history.add(Height(11), TTraits::ToValue(12));
			history.add(Height(22), TTraits::ToValue(98));
			history.add(Height(33), TTraits::ToValue(67));

			// Act:
			history.prune(pruneHeight);

			// Assert:
			checkHistory(history);
		}

		template<typename TTraits>
		auto ZeroValue() {
			return decltype(TTraits::ToValue(0))();
		}
	}

	HISTORY_VALUE_TEST(CanPrune) {
		// Act:
		RunPruneTest<TTraits>(Height(22), [](const auto& history) {
			const auto& valueHistory = TTraits::GetValueHistory(history);

			// Assert:
			EXPECT_EQ(2u, valueHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(22), Height(33) }), valueHistory.heights());
			EXPECT_EQ(TTraits::ToValue(67), valueHistory.get());

			EXPECT_EQ(ZeroValue<TTraits>(), valueHistory.get(Height(11)));
			EXPECT_EQ(ZeroValue<TTraits>(), valueHistory.get(Height(21)));
			EXPECT_EQ(TTraits::ToValue(98), valueHistory.get(Height(22)));
			EXPECT_EQ(TTraits::ToValue(98), valueHistory.get(Height(23)));
			EXPECT_EQ(TTraits::ToValue(67), valueHistory.get(Height(33)));
		});
	}

	TEST(TEST_CLASS, CanPruneHeterogenousValues) {
		// Arrange:
		AccountHistory history;

		history.add(Height(11), Amount(12));
		history.add(Height(22), Amount(98));
		history.add(Height(33), Amount(67));

		history.add(Height(12), Key{ { 100 } });

		history.add(Height(22), VotingPublicKeysTraits::ToValue(75));
		history.add(Height(44), VotingPublicKeysTraits::ToValue(200));

		// Act:
		history.prune(Height(33));

		// Assert:
		{
			const auto& valueHistory = history.balance();
			EXPECT_EQ(1u, valueHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(33) }), valueHistory.heights());
			EXPECT_EQ(Amount(67), valueHistory.get());

			EXPECT_EQ(Amount(67), valueHistory.get(Height(33)));
		}

		{
			const auto& valueHistory = history.vrfPublicKey();
			EXPECT_EQ(1u, valueHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(33) }), valueHistory.heights());
			EXPECT_EQ(Key{ { 100 } }, valueHistory.get());

			EXPECT_EQ(Key{ { 100 } }, valueHistory.get(Height(33)));
		}

		{
			const auto& valueHistory = history.votingPublicKeys();
			EXPECT_EQ(2u, valueHistory.size());
			EXPECT_EQ(std::vector<Height>({ Height(33), Height(44) }), valueHistory.heights());
			EXPECT_EQ(VotingPublicKeysTraits::ToValue(200), valueHistory.get());

			EXPECT_EQ(VotingPublicKeysTraits::ToValue(75), valueHistory.get(Height(33)));
			EXPECT_EQ(VotingPublicKeysTraits::ToValue(200), valueHistory.get(Height(44)));
		}
	}

	// endregion
}}
