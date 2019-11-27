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

#include "catapult/chain/ProcessContextsBuilder.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/observers/ObserverContext.h"
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS ProcessContextsBuilderTests

	namespace {
		// region test context

		struct TestContext {
		public:
			TestContext()
					: Cache(test::CreateCatapultCacheWithMarkerAccount())
					, ResolverCallPairs(0, 0)
					, Builder(Height(111), Timestamp(222), {
						CreateNetworkInfo(),
						[this](const auto& readOnlyCache) {
							++ResolverCallPairs.first;
							if (test::IsMarkedCache(readOnlyCache))
								++ResolverCallPairs.second;

							return test::CreateResolverContextXor();
						}
					})
			{}

		public:
			cache::CatapultCache Cache;
			std::pair<size_t, size_t> ResolverCallPairs;
			ProcessContextsBuilder Builder;

		private:
			static model::NetworkInfo CreateNetworkInfo() {
				model::NetworkInfo networkInfo;
				networkInfo.Identifier = static_cast<model::NetworkIdentifier>(33);
				networkInfo.NodeEqualityStrategy = static_cast<model::NodeIdentityEqualityStrategy>(44);
				return networkInfo;
			}
		};

		// endregion
	}

	// region buildObserverContext

	namespace {
		void AssertObserverContext(const TestContext& context, const observers::ObserverContext& observerContext) {
			// Assert: check basic fields
			EXPECT_EQ(Height(111), observerContext.Height);
			EXPECT_EQ(observers::NotifyMode::Commit, observerContext.Mode);

			// - check resolver wiring
			auto resolveResult = observerContext.Resolvers.resolve(UnresolvedMosaicId(444));
			EXPECT_EQ(1u, context.ResolverCallPairs.first);
			EXPECT_EQ(1u, context.ResolverCallPairs.second);
			EXPECT_EQ(test::CreateResolverContextXor().resolve(UnresolvedMosaicId(444)), resolveResult);

			// - check cache
			EXPECT_TRUE(test::IsMarkedCache(observerContext.Cache));
		}
	}

	TEST(TEST_CLASS, CannotBuildObserverContextWithoutCache) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		EXPECT_THROW(context.Builder.buildObserverContext(), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CannotBuildObserverContextWithCacheView) {
		// Arrange:
		TestContext context;
		auto cacheView = context.Cache.createView();
		context.Builder.setCache(cacheView);

		// Act + Assert:
		EXPECT_THROW(context.Builder.buildObserverContext(), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanBuildObserverContextWithCacheDelta) {
		// Arrange:
		TestContext context;
		auto cacheDelta = context.Cache.createDelta();
		context.Builder.setCache(cacheDelta);

		// Act:
		auto observerContext = context.Builder.buildObserverContext();

		// Assert:
		AssertObserverContext(context, observerContext);
	}

	TEST(TEST_CLASS, CanBuildObserverContextWithCacheDeltaAndBlockStatementBuilder) {
		// Arrange:
		auto blockStatementBuilder = model::BlockStatementBuilder();

		TestContext context;
		auto cacheDelta = context.Cache.createDelta();
		context.Builder.setCache(cacheDelta);
		context.Builder.setBlockStatementBuilder(blockStatementBuilder);

		// Act:
		auto observerContext = context.Builder.buildObserverContext();

		// Assert:
		AssertObserverContext(context, observerContext);

		// - check block statement builder
		observerContext.StatementBuilder().setSource({ 2, 4 });
		EXPECT_EQ(2u, blockStatementBuilder.source().PrimaryId);
		EXPECT_EQ(4u, blockStatementBuilder.source().SecondaryId);
	}

	TEST(TEST_CLASS, CanBuildObserverContextWithObserverState) {
		// Arrange:
		auto blockStatementBuilder = model::BlockStatementBuilder();

		TestContext context;
		auto cacheDelta = context.Cache.createDelta();
		context.Builder.setObserverState(observers::ObserverState(cacheDelta, blockStatementBuilder));

		// Act:
		auto observerContext = context.Builder.buildObserverContext();

		// Assert:
		AssertObserverContext(context, observerContext);

		// - check block statement builder
		observerContext.StatementBuilder().setSource({ 2, 4 });
		EXPECT_EQ(2u, blockStatementBuilder.source().PrimaryId);
		EXPECT_EQ(4u, blockStatementBuilder.source().SecondaryId);
	}

	// endregion

	// region buildValidatorContext

	namespace {
		void AssertValidatorContext(const TestContext& context, const validators::ValidatorContext& validatorContext) {
			// Assert: check basic fields
			EXPECT_EQ(Height(111), validatorContext.Height);
			EXPECT_EQ(Timestamp(222), validatorContext.BlockTime);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(33), validatorContext.Network.Identifier);

			// - check resolver wiring
			auto resolveResult = validatorContext.Resolvers.resolve(UnresolvedMosaicId(444));
			EXPECT_EQ(1u, context.ResolverCallPairs.first);
			EXPECT_EQ(1u, context.ResolverCallPairs.second);
			EXPECT_EQ(test::CreateResolverContextXor().resolve(UnresolvedMosaicId(444)), resolveResult);

			// - check cache
			EXPECT_TRUE(test::IsMarkedCache(validatorContext.Cache));
		}
	}

	TEST(TEST_CLASS, CannotBuildValidatorContextWithoutCache) {
		// Arrange:
		TestContext context;

		// Act + Assert:
		EXPECT_THROW(context.Builder.buildValidatorContext(), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, CanBuildValidatorContextWithCacheView) {
		// Arrange:
		TestContext context;
		auto cacheView = context.Cache.createView();
		context.Builder.setCache(cacheView);

		// Act:
		auto validatorContext = context.Builder.buildValidatorContext();

		// Assert:
		AssertValidatorContext(context, validatorContext);
	}

	TEST(TEST_CLASS, CanBuildValidatorContextWithCacheDelta) {
		// Arrange:
		TestContext context;
		auto cacheDelta = context.Cache.createDelta();
		context.Builder.setCache(cacheDelta);

		// Act:
		auto validatorContext = context.Builder.buildValidatorContext();

		// Assert:
		AssertValidatorContext(context, validatorContext);
	}

	TEST(TEST_CLASS, CanBuildValidatorContextWithObserverState) {
		// Arrange:
		TestContext context;
		auto cacheDelta = context.Cache.createDelta();
		context.Builder.setObserverState(observers::ObserverState(cacheDelta));

		// Act:
		auto validatorContext = context.Builder.buildValidatorContext();

		// Assert:
		AssertValidatorContext(context, validatorContext);
	}

	// endregion
}}
