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

#include "src/state/MosaicRestrictionEvaluator.h"
#include "src/state/MosaicAddressRestriction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicRestrictionEvaluatorTests

	namespace {
		constexpr auto Sentinel_Removal_Value = MosaicAddressRestriction::Sentinel_Removal_Value;

		struct ExpectedEvaluateResults {
			bool Equal;
			bool LessThan;
			bool GreaterThan;
			bool Deleted;
		};

		void RunEvaluateSuccessTest(model::MosaicRestrictionType restrictionType, const ExpectedEvaluateResults& expected) {
			EXPECT_EQ(expected.Equal, EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 123));

			EXPECT_EQ(expected.LessThan, EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 122));
			EXPECT_EQ(expected.LessThan, EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 1));

			EXPECT_EQ(expected.GreaterThan, EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 124));
			EXPECT_EQ(expected.GreaterThan, EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 500));

			EXPECT_EQ(expected.Deleted, EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, Sentinel_Removal_Value));
		}

		void RunEvaluateFailureTest(model::MosaicRestrictionType restrictionType) {
			EXPECT_FALSE(EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 123));

			EXPECT_FALSE(EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 122));
			EXPECT_FALSE(EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 1));

			EXPECT_FALSE(EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 124));
			EXPECT_FALSE(EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, 500));

			EXPECT_FALSE(EvaluateMosaicRestriction({ MosaicId(), 123, restrictionType }, Sentinel_Removal_Value));
		}
	}

	TEST(TEST_CLASS, CanEvaluateRule_EQ) {
		RunEvaluateSuccessTest(model::MosaicRestrictionType::EQ, { true, false, false, false });
	}

	TEST(TEST_CLASS, CanEvaluateRule_NE) {
		RunEvaluateSuccessTest(model::MosaicRestrictionType::NE, { false, true, true, true });
	}

	TEST(TEST_CLASS, CanEvaluateRule_LT) {
		RunEvaluateSuccessTest(model::MosaicRestrictionType::LT, { false, true, false, false });
	}

	TEST(TEST_CLASS, CanEvaluateRule_LE) {
		RunEvaluateSuccessTest(model::MosaicRestrictionType::LE, { true, true, false, false });
	}

	TEST(TEST_CLASS, CanEvaluateRule_GT) {
		RunEvaluateSuccessTest(model::MosaicRestrictionType::GT, { false, false, true, false });
	}

	TEST(TEST_CLASS, CanEvaluateRule_GE) {
		RunEvaluateSuccessTest(model::MosaicRestrictionType::GE, { true, false, true, false });
	}

	TEST(TEST_CLASS, CannotEvaluateRule_NONE) {
		RunEvaluateFailureTest(model::MosaicRestrictionType::NONE);
	}

	TEST(TEST_CLASS, CannotEvaluateRule_Other) {
		RunEvaluateFailureTest(static_cast<model::MosaicRestrictionType>(128));
	}
}}
