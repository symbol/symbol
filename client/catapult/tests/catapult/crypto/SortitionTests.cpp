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

#include "catapult/crypto/Sortition.h"
#include "catapult/utils/Functional.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS SortitionTests

	TEST(TEST_CLASS, InverseCdf_RoundsResultUp) {
		// Arrange:
		// (10, p) binomial distribution
		// p = 0.3:
		//   CDF |0.0282|0.1493|0.3828|0.6496|0.8497|0.9527|0.9894|0.9984|0.9999|1.0   |1.0   |
		//   PMF |0.028 |0.121 |0.233 |0.267 |0.200 |0.103 |0.037 |0.009 |0.001 |0.000 |0.000 |
		// p = 0.5
		//   CDF |0.0010|0.0107|0.0547|0.1719|0.3770|0.6230|0.8281|0.9453|0.9893|0.9990|1.0   |
		//   PMF |0.001 |0.010 |0.044 |0.117 |0.205 |0.246 |0.205 |0.117 |0.044 |0.010 |0.001 |
		// p = 0.7:
		//   CDF |~.0000|0.0001|0.0015|0.0015|0.0473|0.1502|0.3503|0.6172|0.8506|0.9717|1.0   |
		//   PMF |~.0000|0.0001|0.001 |0.009 |0.036 |0.010 |0.200 |0.266 |0.233 |0.121 |0.028 |
		//       | 0    | 1    | 2    | 3    | 4    | 5    | 6    | 7    | 8    | 9    | 10   |
		//
		// Aligned for 0.64, p: 0.3 | 0.5 | 0.7
		//    expected results:   3 |   6 |   8

		// Act + Assert:
		EXPECT_EQ(3u, InverseCdf(10, 0.3, 0.64));
		EXPECT_EQ(6u, InverseCdf(10, 0.5, 0.64));
		EXPECT_EQ(8u, InverseCdf(10, 0.7, 0.64));
	}

	namespace {
		// generate random amounts
		auto GenerateAmounts(size_t numAccounts) {
			std::vector<Amount> amounts;
			amounts.resize(numAccounts);

			std::generate(amounts.begin(), amounts.end(), []() {
				return Amount(200'000'000 + test::Random() % 100'000'000);
			});

			return amounts;
		}
	}

	TEST(TEST_CLASS, Sortition_Sample_Data) {
		// Arrange:
		constexpr auto Committee_Threshold = 2117u;
		constexpr auto Committee_Tau = 2990u;

		auto amounts = GenerateAmounts(2000);
		auto totalPower = utils::Sum(amounts, [](auto amount) { return amount; });

		// Act:
		uint64_t collectedThreshold = 0;
		auto votingAccounts = 0u;
		for (const auto& amount : amounts) {
			// for test purposes there is no need to generate vrf hash,
			// generating random hash per user is ok
			auto vrfHash = test::GenerateRandomByteArray<Hash512>();

			auto value = Sortition(vrfHash, Committee_Tau, amount, totalPower);
			collectedThreshold += value;
			if (value)
				++votingAccounts;
		}

		// Assert:
		EXPECT_GT(collectedThreshold, Committee_Threshold);
		CATAPULT_LOG(debug) << "collected threshold " << collectedThreshold;
		CATAPULT_LOG(debug) << "voting accounts " << votingAccounts;
	}
}}
