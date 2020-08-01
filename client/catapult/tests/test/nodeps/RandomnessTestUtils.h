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

#pragma once
#include "catapult/utils/Casting.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Random generator tests.
	class RandomnessTestUtils {
	public:
		template<typename TGenerator>
		static void AssertRandomGeneratorUint64() {
			// Act + Assert:
			EXPECT_EQ(0u, TGenerator::min());
			EXPECT_EQ(std::numeric_limits<uint64_t>::max(), TGenerator::max());
		}

	private:
		static constexpr auto Num_Samples = 10'000u;
		static constexpr auto Num_Buckets = 16u; // evenly size buckets

	private:
		static double CalculateChiSquare(const std::array<uint64_t, Num_Buckets>& buckets, uint64_t expectedValue) {
			auto chiSquare = 0.0;
			auto minValue = expectedValue;
			auto maxValue = expectedValue;
			for (auto observedValue : buckets) {
				minValue = std::min(observedValue, minValue);
				maxValue = std::max(observedValue, maxValue);

				auto difference = observedValue - expectedValue;
				chiSquare += utils::to_ratio(difference * difference, expectedValue);
			}

			CATAPULT_LOG(debug) << "chiSquare = " << chiSquare << ", min = " << minValue << ", max = " << maxValue;
			return chiSquare;
		}

		static double LookupProbability(double chiSquare) {
			auto chiSquareTable = std::initializer_list<std::pair<double, double>>{
				// df = 15
				{ 37.697, 1 - 0.001 },
				{ 35.628, 1 - 0.002 },
				{ 32.801, 1 - 0.005 },
				{ 30.578, 1 - 0.010 },
				{ 27.488, 1 - 0.025 },
				{ 24.996, 1 - 0.050 },
				{ 22.307, 1 - 0.100 },
				{ 18.245, 1 - 0.250 },
				{ 14.339, 1 - 0.500 },
				{ 11.037, 1 - 0.750 },
				{ 8.5470, 1 - 0.900 },
				{ 7.2610, 1 - 0.950 },
				{ 6.2620, 1 - 0.975 },
				{ 5.2290, 1 - 0.990 },
				{ 4.6010, 1 - 0.995 }
			};

			for (const auto& pair : chiSquareTable) {
				if (chiSquare > pair.first) {
					auto probability = pair.second * 100;
					CATAPULT_LOG(debug) << "randomness hypothesis can be rejected with at least " << probability << " percent certainty";
					return probability;
				}
			}

			return 100;
		}

		template<typename TValue>
		static size_t GetBucketIndex(TValue value) {
			return value / (std::numeric_limits<TValue>::max() / Num_Buckets + 1);
		}

	public:
		template<typename TGenerator>
		static void AssertExhibitsRandomness() {
			// Assert: non-deterministic because testing randomness
			RunNonDeterministicTest("AssertExhibitsRandomness", []() {
				// Arrange:
				TGenerator generator;

				// Act:
				std::array<uint64_t, Num_Buckets> buckets{};
				for (auto i = 0u; i < Num_Samples; ++i) {
					auto value = generator();
					++buckets[GetBucketIndex(value)];
				}

				// Assert:
				auto chiSquare = CalculateChiSquare(buckets, Num_Samples / Num_Buckets);
				auto probability = LookupProbability(chiSquare);
				return probability < 75.0;
			});
		}

		template<typename TGenerator>
		static void AssertFillExhibitsRandomness() {
			// Assert: non-deterministic because testing randomness
			RunNonDeterministicTest("AssertFillExhibitsRandomness", []() {
				// Arrange:
				TGenerator generator;

				// Act:
				std::array<uint64_t, Num_Buckets> buckets{};
				for (auto i = 0u; i < Num_Samples / 20; ++i) {
					std::array<uint8_t, 20> values;
					generator.fill(values.data(), values.size());

					for (auto value : values)
						++buckets[GetBucketIndex(value)];
				}

				// Assert:
				auto chiSquare = CalculateChiSquare(buckets, Num_Samples / Num_Buckets);
				auto probability = LookupProbability(chiSquare);
				return probability < 75.0;
			});
		}
	};

/// Adds all randomness tests for a uint64 random generator named \a NAME.
#define DEFINE_RANDOMNESS_UINT64_TESTS(NAME) \
	TEST(TEST_CLASS, NAME##ExposesPropertiesForUint64) { \
		test::RandomnessTestUtils::AssertRandomGeneratorUint64<NAME>(); \
	} \
	TEST(TEST_CLASS, NAME##ExhibitsRandomness) { \
		test::RandomnessTestUtils::AssertExhibitsRandomness<NAME>(); \
	} \
	TEST(TEST_CLASS, NAME##FillExhibitsRandomness) { \
		test::RandomnessTestUtils::AssertFillExhibitsRandomness<NAME>(); \
	}
}}
