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

#include "catapult/cache/IdentifierGroupCacheUtils.h"
#include "tests/test/cache/TestCacheTypes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace cache {

#define TEST_CLASS BaseSetDeltaIntegrityTests

	namespace {
		using TestCacheDescriptor = test::TestCacheTypes::TestCacheDescriptor;
		using BaseSetType = test::TestCacheTypes::BaseSetType;
		using BaseSetDeltaType = test::TestCacheTypes::BaseSetDeltaType;

		constexpr auto Element = "Alice";

		enum class DeltaSetAction { No_Operation, Increment_Generation, Find, Find_Const, Insert, Remove, Commit };

		void PerformDeltaSetAction(BaseSetType& set, BaseSetDeltaType& deltaSet, DeltaSetAction action) {
			switch (action) {
			case DeltaSetAction::No_Operation:
				break;

			case DeltaSetAction::Increment_Generation:
				deltaSet.incrementGenerationId();
				break;

			case DeltaSetAction::Find:
				deltaSet.find(TestCacheDescriptor::GetKeyFromValue(Element));
				break;

			case DeltaSetAction::Find_Const:
				const_cast<const BaseSetDeltaType&>(deltaSet).find(TestCacheDescriptor::GetKeyFromValue(Element));
				break;

			case DeltaSetAction::Insert:
				deltaSet.insert(Element);
				break;

			case DeltaSetAction::Remove:
				deltaSet.remove(TestCacheDescriptor::GetKeyFromValue(Element));
				break;

			case DeltaSetAction::Commit:
				set.commit();
				break;
			}
		}

		void RunGenerationIdTest(uint32_t expectedGenerationId, const std::vector<DeltaSetAction>& deltaSetActions) {
			// Arrange:
			BaseSetType set;
			auto pDeltaSet = set.rebase();
			pDeltaSet->insert(Element);

			// Act:
			for (auto action : deltaSetActions)
				PerformDeltaSetAction(set, *pDeltaSet, action);

			// Assert:
			EXPECT_EQ(expectedGenerationId, pDeltaSet->generationId(TestCacheDescriptor::GetKeyFromValue(Element)));
		}
	}

	// region single delta set action type

	TEST(TEST_CLASS, GenerationIdIsCorrect_All_No_Operation) {
		RunGenerationIdTest(1, std::vector<DeltaSetAction>(5, DeltaSetAction::No_Operation));
	}

	TEST(TEST_CLASS, GenerationIdIsCorrect_All_Increment_Generation) {
		RunGenerationIdTest(1, std::vector<DeltaSetAction>(5, DeltaSetAction::Increment_Generation));
	}

	TEST(TEST_CLASS, GenerationIdIsCorrect_All_Find) {
		RunGenerationIdTest(1, std::vector<DeltaSetAction>(5, DeltaSetAction::Find));
	}

	TEST(TEST_CLASS, GenerationIdIsCorrect_All_Find_Const) {
		RunGenerationIdTest(1, std::vector<DeltaSetAction>(5, DeltaSetAction::Find_Const));
	}

	TEST(TEST_CLASS, GenerationIdIsCorrect_All_Insert) {
		RunGenerationIdTest(1, std::vector<DeltaSetAction>(5, DeltaSetAction::Insert));
	}

	TEST(TEST_CLASS, GenerationIdIsCorrect_All_Remove) {
		RunGenerationIdTest(1, std::vector<DeltaSetAction>(5, DeltaSetAction::Remove));
	}

	TEST(TEST_CLASS, GenerationIdIsCorrect_All_Commit) {
		RunGenerationIdTest(0, std::vector<DeltaSetAction>(5, DeltaSetAction::Commit));
	}

	// endregion

	// region mixed delta set action types

	TEST(TEST_CLASS, GenerationIdIsCorrect_Intermediate_Commit) {
		RunGenerationIdTest(0, {
			DeltaSetAction::Insert,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Find,
			DeltaSetAction::Commit,
			DeltaSetAction::No_Operation
		});
	}

	TEST(TEST_CLASS, GenerationIdIsCorrect_Intermediate_Increment) {
		RunGenerationIdTest(2, {
			DeltaSetAction::No_Operation,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Remove,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Find
		});

		RunGenerationIdTest(3, {
			DeltaSetAction::No_Operation,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::No_Operation,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Remove
		});

		RunGenerationIdTest(2, {
			DeltaSetAction::No_Operation,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Commit,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Remove
		});

		RunGenerationIdTest(3, {
			DeltaSetAction::No_Operation,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Find,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Find
		});

		RunGenerationIdTest(1, {
			DeltaSetAction::No_Operation,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Find_Const,
			DeltaSetAction::Increment_Generation,
			DeltaSetAction::Find_Const
		});
	}

	// endregion
}}
