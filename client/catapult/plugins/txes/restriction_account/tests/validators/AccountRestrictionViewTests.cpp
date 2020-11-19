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

#include "src/validators/Validators.h"
#include "src/validators/AccountRestrictionView.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "tests/test/AccountRestrictionCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountRestrictionViewTests

	namespace {
		struct PopulateOptions {
		public:
			PopulateOptions(size_t numRestrictions)
					: NumRestrictions(numRestrictions)
					, ShouldBlock(false)
			{}

		public:
			size_t NumRestrictions;
			bool ShouldBlock;
		};

		auto PopulateCache(const PopulateOptions& options, cache::CatapultCache& cache) {
			auto delta = cache.createDelta();
			auto& restrictionCacheDelta = delta.sub<cache::AccountRestrictionCache>();
			auto address = test::GenerateRandomByteArray<Address>();
			auto restrictions = state::AccountRestrictions(address);
			auto& restriction = restrictions.restriction(model::AccountRestrictionFlags::MosaicId);
			for (auto i = 0u; i < options.NumRestrictions; ++i) {
				auto modification = model::AccountRestrictionModification{
					model::AccountRestrictionModificationAction::Add,
					state::ToVector(MosaicId(i))
				};
				if (options.ShouldBlock)
					restriction.block(modification);
				else
					restriction.allow(modification);
			}

			restrictionCacheDelta.insert(restrictions);
			cache.commit(Height(1));
			return address;
		}

		template<typename TAction>
		void RunTest(const PopulateOptions& options, TAction action) {
			// Arrange:
			auto cache = test::AccountRestrictionCacheFactory::Create();
			auto address = PopulateCache(options, cache);
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			AccountRestrictionView view(readOnlyCache);

			// Act:
			action(view, address);
		}
	}

	// region initialize

	TEST(TEST_CLASS, InitializeReturnsFalseWhenAddressIsUnknown) {
		// Arrange:
		RunTest(5, [](auto& view, const auto&) {
			// Act + Assert:
			EXPECT_FALSE(view.initialize(test::GenerateRandomByteArray<Address>()));
		});
	}

	TEST(TEST_CLASS, InitializeReturnsTrueWhenAddressIsKnown) {
		// Arrange:
		RunTest(5, [](auto& view, const auto& address) {
			// Act + Assert:
			EXPECT_TRUE(view.initialize(address));
		});
	}

	// endregion

	// region get

	TEST(TEST_CLASS, GetReturnsExpectedAccountRestriction) {
		// Arrange:
		RunTest(5, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			const auto& restriction = view.get(model::AccountRestrictionFlags::MosaicId);

			// Assert:
			EXPECT_EQ(5u, restriction.values().size());
			for (auto i = 0u; i < 5; ++i)
				EXPECT_TRUE(restriction.contains(state::ToVector(MosaicId(i))));
		});
	}

	TEST(TEST_CLASS, GetThrowsWhenInitializeFailed) {
		// Arrange:
		RunTest(5, [](auto& view, const auto&) {
			ASSERT_FALSE(view.initialize(test::GenerateRandomByteArray<Address>()));

			// Act + Assert:
			EXPECT_THROW(view.get(model::AccountRestrictionFlags::MosaicId), catapult_invalid_argument);
		});
	}

	// endregion

	// region isAllowed

	TEST(TEST_CLASS, IsAllowedReturnsTrueWhenAccountRestrictionIsEmpty) {
		// Arrange:
		RunTest(0, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::AccountRestrictionFlags::MosaicId, MosaicId(10));

			// Assert:
			EXPECT_TRUE(isAllowed);
		});
	}

	TEST(TEST_CLASS, IsAllowedReturnsTrueWhenOperationTypeIsAllowAndValueIsContained) {
		// Arrange:
		RunTest(5, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::AccountRestrictionFlags::MosaicId, MosaicId(3));

			// Assert:
			EXPECT_TRUE(isAllowed);
		});
	}

	TEST(TEST_CLASS, IsAllowedReturnsFalseWhenOperationTypeIsAllowAndValueIsNotContained) {
		// Arrange:
		RunTest(5, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::AccountRestrictionFlags::MosaicId, MosaicId(7));

			// Assert:
			EXPECT_FALSE(isAllowed);
		});
	}

	TEST(TEST_CLASS, IsAllowedReturnsTrueWhenOperationTypeIsBlockAndValueIsNotContained) {
		// Arrange:
		auto options = PopulateOptions(5);
		options.ShouldBlock = true;
		RunTest(options, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::AccountRestrictionFlags::MosaicId, MosaicId(7));

			// Assert:
			EXPECT_TRUE(isAllowed);
		});
	}

	TEST(TEST_CLASS, IsAllowedReturnsFalseWhenOperationTypeIsBlockAndValueIsContained) {
		// Arrange:
		auto options = PopulateOptions(5);
		options.ShouldBlock = true;
		RunTest(options, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::AccountRestrictionFlags::MosaicId, MosaicId(3));

			// Assert:
			EXPECT_FALSE(isAllowed);
		});
	}

	// endregion
}}
