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

#pragma once
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	/// Lock cache unique validator test suite.
	template<typename TTraits>
	struct LockCacheUniqueValidatorTests {
	private:
		using BasicTraits = typename TTraits::DescriptorType;

	public:
		static void AssertSuccessWhenHashIsNotInCache() {
			// Arrange:
			auto cache = CreateDefaultCache(test::CreateLockInfos<BasicTraits>(3));
			typename TTraits::NotificationBuilder builder;

			// Assert:
			RunTest(ValidationResult::Success, builder.notification(), cache, Height(100));
		}

		static void AssertSuccessWhenHashIsInCacheAndInactive() {
			RunNotEmptyCacheTest(ValidationResult::Success, Height(11), Height(11));
		}

		static void AssertFailureWhenHashIsInCacheAndActive() {
			RunNotEmptyCacheTest(TTraits::Failure, Height(11), Height(10));
		}

	private:
		static auto CreateDefaultCache(const std::vector<typename BasicTraits::ValueType>& lockInfos) {
			using CacheFactory = typename TTraits::CacheFactory;
			auto cache = CacheFactory::Create();
			{
				auto cacheDelta = cache.createDelta();
				auto& lockInfoCacheDelta = cacheDelta.template sub<typename BasicTraits::CacheType>();
				for (const auto& lockInfo : lockInfos)
					lockInfoCacheDelta.insert(lockInfo);

				cache.commit(Height());
			}

			return cache;
		}

	public:
		static void RunNotEmptyCacheTest(ValidationResult expectedResult, Height endHeight, Height notificationHeight) {
			// Arrange:
			auto lockInfos = test::CreateLockInfos<BasicTraits>(3);
			lockInfos[1].Status = state::LockStatus::Unused;
			lockInfos[1].EndHeight = endHeight;
			auto cache = CreateDefaultCache(lockInfos);

			typename TTraits::NotificationBuilder builder;
			builder.prepare(lockInfos[1]);

			// Assert:
			RunTest(expectedResult, builder.notification(), cache, notificationHeight);
		}

		template<typename TCache>
		static void RunTest(
				ValidationResult expectedResult,
				const typename TTraits::NotificationType& notification,
				const TCache& cache,
				Height height) {
			// Arrange:
			auto pValidator = TTraits::CreateValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache, height);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	};
}}

#define MAKE_CACHE_UNIQUE_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockCacheUniqueValidatorTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_CACHE_UNIQUE_TESTS(TRAITS_NAME) \
	MAKE_CACHE_UNIQUE_TEST(TRAITS_NAME, SuccessWhenHashIsNotInCache) \
	MAKE_CACHE_UNIQUE_TEST(TRAITS_NAME, SuccessWhenHashIsInCacheAndInactive) \
	MAKE_CACHE_UNIQUE_TEST(TRAITS_NAME, FailureWhenHashIsInCacheAndActive)
