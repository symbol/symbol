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
#include "LockInfoCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Lock cache unique test suite.
	template<typename TTraits>
	struct LockCacheUniqueTests {
	private:
		using BasicTraits = typename TTraits::DescriptorType;

	public:
		static void AssertSuccessWhenHashIsNotInCache() {
			// Arrange:
			typename TTraits::NotificationBuilder builder;
			auto cache = CreateDefaultCache(test::CreateLockInfos<BasicTraits>(3));

			// Assert:
			RunTest(validators::ValidationResult::Success, builder.notification(), cache);
		}

		static void AssertFailureWhenHashIsInCache() {
			// Arrange:
			typename TTraits::NotificationBuilder builder;
			auto infos = test::CreateLockInfos<BasicTraits>(3);
			auto cache = CreateDefaultCache(infos);
			builder.setHash(BasicTraits::ToKey(infos[1]));

			// Assert:
			RunTest(TTraits::Failure, builder.notification(), cache);
		}
	private:
		static auto CreateDefaultCache(const std::vector<typename BasicTraits::ValueType>& infos) {
			using CacheFactory = typename TTraits::CacheFactory;
			auto cache = CacheFactory::Create();
			{
				auto cacheDelta = cache.createDelta();
				auto& lockInfoCacheDelta = cacheDelta.template sub<typename BasicTraits::CacheType>();
				for (const auto& info : infos)
					lockInfoCacheDelta.insert(info);

				cache.commit(Height());
			}
			return cache;
		}

		template<typename TCache>
		static void RunTest(
				validators::ValidationResult expectedResult,
				const typename TTraits::NotificationType& notification,
				const TCache& cache) {
			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(123), readOnlyCache);

			auto pValidator = TTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	};
}}

#define MAKE_CACHE_UNIQUE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::LockCacheUniqueTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_CACHE_UNIQUE_TESTS(TRAITS_NAME) \
	MAKE_CACHE_UNIQUE_TEST(TRAITS_NAME, SuccessWhenHashIsNotInCache) \
	MAKE_CACHE_UNIQUE_TEST(TRAITS_NAME, FailureWhenHashIsInCache)
