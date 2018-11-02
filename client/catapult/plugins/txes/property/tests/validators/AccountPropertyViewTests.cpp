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
#include "src/validators/AccountPropertyView.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "tests/test/PropertyCacheTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountPropertyViewTests

	namespace {
		struct PopulateOptions {
		public:
			PopulateOptions(size_t numProperties)
					: NumProperties(numProperties)
					, ShouldBlock(false)
			{}

		public:
			size_t NumProperties;
			bool ShouldBlock;
		};

		auto PopulateCache(const PopulateOptions& options, cache::CatapultCache& cache) {
			auto delta = cache.createDelta();
			auto& propertyCacheDelta = delta.sub<cache::PropertyCache>();
			auto address = test::GenerateRandomData<Address_Decoded_Size>();
			auto accountProperties = state::AccountProperties(address);
			auto& accountProperty = accountProperties.property(model::PropertyType::MosaicId);
			for (auto i = 0u; i < options.NumProperties; ++i) {
				auto modification = model::RawPropertyModification{ model::PropertyModificationType::Add, state::ToVector(MosaicId(i)) };
				if (options.ShouldBlock)
					accountProperty.block(modification);
				else
					accountProperty.allow(modification);
			}

			propertyCacheDelta.insert(accountProperties);
			cache.commit(Height(1));
			return address;
		}

		template<typename TAction>
		void RunTest(const PopulateOptions& options, TAction action) {
			// Arrange:
			auto cache = test::PropertyCacheFactory::Create();
			auto address = PopulateCache(options, cache);
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			AccountPropertyView view(readOnlyCache);

			// Act:
			action(view, address);
		}
	}

	// region initialize

	TEST(TEST_CLASS, InitializeReturnsFalseWhenAddressIsUnknown) {
		// Arrange:
		RunTest(5, [](auto& view, const auto&) {
			// Act + Assert:
			EXPECT_FALSE(view.initialize(test::GenerateRandomData<Address_Decoded_Size>()));
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

	TEST(TEST_CLASS, GetReturnsExpectedTypedProperty) {
		// Arrange:
		RunTest(5, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto typedProperty = view.template get<MosaicId>(model::PropertyType::MosaicId);

			// Assert:
			EXPECT_EQ(5u, typedProperty.size());
			for (auto i = 0u; i < 5; ++i)
				EXPECT_TRUE(typedProperty.contains(MosaicId(i)));
		});
	}

	TEST(TEST_CLASS, GetThrowsWhenInitializeFailed) {
		// Arrange:
		RunTest(5, [](auto& view, const auto&) {
			ASSERT_FALSE(view.initialize(test::GenerateRandomData<Address_Decoded_Size>()));

			// Act + Assert:
			EXPECT_THROW(view.template get<MosaicId>(model::PropertyType::MosaicId), catapult_invalid_argument);
		});
	}

	// endregion

	// region isAllowed

	TEST(TEST_CLASS, IsAllowedReturnsTrueWhenPropertyIsEmpty) {
		// Arrange:
		RunTest(0, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::PropertyType::MosaicId, MosaicId(10));

			// Assert:
			EXPECT_TRUE(isAllowed);
		});
	}

	TEST(TEST_CLASS, IsAllowedReturnsTrueWhenOperationTypeIsAllowAndValueIsContained) {
		// Arrange:
		RunTest(5, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::PropertyType::MosaicId, MosaicId(3));

			// Assert:
			EXPECT_TRUE(isAllowed);
		});
	}

	TEST(TEST_CLASS, IsAllowedReturnsFalseWhenOperationTypeIsAllowAndValueIsNotContained) {
		// Arrange:
		RunTest(5, [](auto& view, const auto& address) {
			ASSERT_TRUE(view.initialize(address));

			// Act:
			auto isAllowed = view.isAllowed(model::PropertyType::MosaicId, MosaicId(7));

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
			auto isAllowed = view.isAllowed(model::PropertyType::MosaicId, MosaicId(7));

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
			auto isAllowed = view.isAllowed(model::PropertyType::MosaicId, MosaicId(3));

			// Assert:
			EXPECT_FALSE(isAllowed);
		});
	}

	// endregion
}}
