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

#include "catapult/observers/ObserverStatementBuilder.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS ObserverStatementBuilderTests

	namespace {
		void PrepareRandomReceipt(model::Receipt& receipt) {
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&receipt), sizeof(model::Receipt) });
			receipt.Size = sizeof(model::Receipt);
		}
	}

	// region ObserverStatementBuilder - capture disabled

	TEST(TEST_CLASS, BuilderCaptureDisabled_SourceIsInitiallyZeroed) {
		// Arrange:
		ObserverStatementBuilder builder;

		// Act:
		auto source = builder.source();

		// Assert:
		EXPECT_EQ(0u, source.PrimaryId);
		EXPECT_EQ(0u, source.SecondaryId);
	}

	TEST(TEST_CLASS, BuilderCaptureDisabled_CannotChangeSource) {
		// Arrange:
		ObserverStatementBuilder builder;

		// Act:
		builder.setSource({ 12, 11 });
		auto source = builder.source();

		// Assert:
		EXPECT_EQ(0u, source.PrimaryId);
		EXPECT_EQ(0u, source.SecondaryId);
	}

	TEST(TEST_CLASS, BuilderCaptureDisabled_CanAddReceipt) {
		// Arrange:
		model::Receipt receipt;
		PrepareRandomReceipt(receipt);

		ObserverStatementBuilder builder;

		// Act:
		builder.setSource({ 12, 11 });
		builder.addReceipt(receipt);

		// Assert: no exception
	}

	// endregion

	// region ObserverStatementBuilder - capture enabled

	TEST(TEST_CLASS, BuilderCaptureEnabled_SourceIsInitiallyZeroed) {
		// Arrange:
		model::BlockStatementBuilder blockStatementBuilder;
		ObserverStatementBuilder builder(blockStatementBuilder);

		// Act:
		auto source = builder.source();

		// Assert:
		EXPECT_EQ(0u, source.PrimaryId);
		EXPECT_EQ(0u, source.SecondaryId);
	}

	TEST(TEST_CLASS, BuilderCaptureEnabled_CanChangeSource) {
		// Arrange:
		model::BlockStatementBuilder blockStatementBuilder;
		ObserverStatementBuilder builder(blockStatementBuilder);

		// Act:
		builder.setSource({ 12, 11 });
		auto source = builder.source();

		// Assert:
		EXPECT_EQ(12u, source.PrimaryId);
		EXPECT_EQ(11u, source.SecondaryId);
	}

	TEST(TEST_CLASS, BuilderCaptureEnabled_CanAddReceipt) {
		// Arrange:
		model::Receipt receipt;
		PrepareRandomReceipt(receipt);

		model::BlockStatementBuilder blockStatementBuilder;
		ObserverStatementBuilder builder(blockStatementBuilder);

		// Act:
		builder.setSource({ 12, 11 });
		builder.addReceipt(receipt);
		auto pStatement = blockStatementBuilder.build();

		// Assert:
		ASSERT_EQ(1u, pStatement->TransactionStatements.size());

		model::TransactionStatement transactionStatement({ 12, 11 });
		transactionStatement.addReceipt(receipt);
		EXPECT_EQ(transactionStatement.hash(), pStatement->TransactionStatements.find({ 12, 11 })->second.hash());
	}

	// endregion

	// region Bind

	namespace {
		model::ResolverContext CreateConditionalResolverContext() {
			auto resolver = [](auto shouldXorResolve, const auto& unresolved) {
				return (shouldXorResolve ? test::CreateResolverContextXor() : model::ResolverContext()).resolve(unresolved);
			};
			return model::ResolverContext(
					[resolver](const auto& unresolved) { return resolver(0 != unresolved.unwrap() % 2, unresolved); },
					[resolver](const auto& unresolved) { return resolver(0 != unresolved[0] % 2, unresolved); });
		}

		struct AddressResolverTraits {
			using StatementType = model::AddressResolutionStatement;

			static auto CreateUnresolved(uint8_t id) {
				return UnresolvedAddress{ { { id } } };
			}

			static const auto& GetStatements(const model::BlockStatement& blockStatement) {
				return blockStatement.AddressResolutionStatements;
			}
		};

		struct MosaicResolverTraits {
			using StatementType = model::MosaicResolutionStatement;

			static auto CreateUnresolved(uint8_t id) {
				return UnresolvedMosaicId(id);
			}

			static const auto& GetStatements(const model::BlockStatement& blockStatement) {
				return blockStatement.MosaicResolutionStatements;
			}
		};
	}

#define RESOLVER_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressResolverTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicResolverTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	RESOLVER_BASED_TEST(CanBindAndCaptureResolver_AddsStatementWhenResolveRequired) {
		// Arrange:
		auto unresolved = TTraits::CreateUnresolved(111);
		auto originalResolverContext = CreateConditionalResolverContext();
		model::BlockStatementBuilder blockStatementBuilder;
		blockStatementBuilder.setSource({ 10, 8 });

		// Act:
		auto resolverContext = Bind(originalResolverContext, blockStatementBuilder);
		auto resolved = resolverContext.resolve(unresolved);
		auto pStatement = blockStatementBuilder.build();

		// Assert:
		EXPECT_EQ(unresolved, test::UnresolveXor(resolved));
		ASSERT_EQ(1u, TTraits::GetStatements(*pStatement).size());

		typename TTraits::StatementType resolutionStatement(unresolved);
		resolutionStatement.addResolution(resolved, { 10, 8 });
		EXPECT_EQ(resolutionStatement.hash(), TTraits::GetStatements(*pStatement).find(unresolved)->second.hash());
	}

	RESOLVER_BASED_TEST(CanBindAndCaptureResolver_DoesNotAddStatementWhenResolveNotRequired) {
		// Arrange:
		auto unresolved = TTraits::CreateUnresolved(222);
		auto originalResolverContext = CreateConditionalResolverContext();
		model::BlockStatementBuilder blockStatementBuilder;
		blockStatementBuilder.setSource({ 10, 8 });

		// Act:
		auto resolverContext = Bind(originalResolverContext, blockStatementBuilder);
		auto resolved = resolverContext.resolve(unresolved);
		auto pStatement = blockStatementBuilder.build();

		// Assert:
		EXPECT_EQ(model::ResolverContext().resolve(unresolved), resolved);
		ASSERT_EQ(0u, TTraits::GetStatements(*pStatement).size());
	}

	// endregion
}}
