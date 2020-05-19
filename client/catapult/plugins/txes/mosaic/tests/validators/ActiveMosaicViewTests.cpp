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

#include "src/validators/Validators.h"
#include "src/validators/ActiveMosaicView.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS ActiveMosaicViewTests

	namespace {
		struct TryGetTraits {
			static validators::ValidationResult TryGet(const ActiveMosaicView& view, MosaicId id, Height height) {
				ActiveMosaicView::FindIterator mosaicIter;
				return view.tryGet(id, height, mosaicIter);
			}
		};

		struct TryGetWithOwnerTraits {
			static validators::ValidationResult TryGet(const ActiveMosaicView& view, MosaicId id, Height height) {
				ActiveMosaicView::FindIterator mosaicIter;
				return view.tryGet(id, height, Address(), mosaicIter);
			}
		};
	}

#define TRY_GET_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryGetTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_WithOwner) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TryGetWithOwnerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region tryGet - active checks

	TRY_GET_BASED_TEST(CannotGetUnknownMosaic) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act:
		auto result = TTraits::TryGet(view, MosaicId(123), Height(100));

		// Assert:
		EXPECT_EQ(Failure_Mosaic_Expired, result);
	}

	TRY_GET_BASED_TEST(CannotGetInactiveMosaic) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		test::AddMosaic(delta, MosaicId(123), Height(50), BlockDuration(100), Amount());
		cache.commit(Height());

		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act: mosaic expires at 50 + 100
		auto result = TTraits::TryGet(view, MosaicId(123), Height(200));

		// Assert:
		EXPECT_EQ(Failure_Mosaic_Expired, result);
	}

	// endregion

	// region tryGet - owner checks

	template<typename TAction>
	void RunTestWithActiveMosaic(const Address& owner, TAction action) {
		// Arrange:
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		test::AddEternalMosaic(delta, MosaicId(123), Height(50), owner);
		cache.commit(Height());

		auto readOnlyCache = delta.toReadOnly();
		auto view = ActiveMosaicView(readOnlyCache);

		// Act + Assert:
		action(view);
	}

	TEST(TEST_CLASS, CanGetActiveMosaicWithoutOwnerCheck) {
		// Arrange:
		RunTestWithActiveMosaic(test::CreateRandomOwner(), [](const auto& view) {
			// Act:
			ActiveMosaicView::FindIterator mosaicIter;
			auto result = view.tryGet(MosaicId(123), Height(100), mosaicIter);

			// Assert:
			EXPECT_EQ(ValidationResult::Success, result);
			EXPECT_EQ(MosaicId(123), mosaicIter.get().mosaicId());
		});
	}

	TEST(TEST_CLASS, CannotGetActiveMosaicWithWrongOwner) {
		// Arrange:
		RunTestWithActiveMosaic(test::CreateRandomOwner(), [](const auto& view) {
			// Act:
			ActiveMosaicView::FindIterator mosaicIter;
			auto result = view.tryGet(MosaicId(123), Height(100), test::CreateRandomOwner(), mosaicIter);

			// Assert:
			EXPECT_EQ(Failure_Mosaic_Owner_Conflict, result);
		});
	}

	TEST(TEST_CLASS, CanGetActiveMosaicWithCorrectOwner) {
		// Arrange:
		const auto& owner = test::CreateRandomOwner();
		RunTestWithActiveMosaic(owner, [&owner](const auto& view) {
			// Act:
			ActiveMosaicView::FindIterator mosaicIter;
			auto result = view.tryGet(MosaicId(123), Height(100), owner, mosaicIter);

			// Assert:
			EXPECT_EQ(ValidationResult::Success, result);
			EXPECT_EQ(MosaicId(123), mosaicIter.get().mosaicId());
		});
	}

	// endregion
}}
