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

#include "catapult/model/Resolvable.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS ResolvableTests

	// region traits

	namespace {
		struct AddressTraits {
			using ResolvableType = ResolvableAddress;
			using ResolvedType = Address;
			using UnresolvedType = UnresolvedAddress;

			static constexpr auto Unresolve = extensions::CopyToUnresolvedAddress;

			static ResolvedType GenerateRandomResolvedValue() {
				return test::GenerateRandomByteArray<ResolvedType>();
			}
		};

		struct MosaicTraits {
			using ResolvableType = ResolvableMosaicId;
			using ResolvedType = MosaicId;
			using UnresolvedType = UnresolvedMosaicId;

			static constexpr auto Unresolve = extensions::CastToUnresolvedMosaicId;

			static ResolvedType GenerateRandomResolvedValue() {
				return test::GenerateRandomValue<ResolvedType>();
			}
		};
	}

#define RESOLVABLE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region test utils

	namespace {
		template<typename TTraits>
		typename TTraits::ResolvedType ThrowingResolver(const typename TTraits::UnresolvedType& unresolved) {
			CATAPULT_THROW_INVALID_ARGUMENT_1("throwing resolver cannot resolve value", unresolved);
		}

		model::ResolverContext CreateThrowingResolverContext() {
			return model::ResolverContext(ThrowingResolver<MosaicTraits>, ThrowingResolver<AddressTraits>);
		}
	}

	// endregion

	// region tests

	RESOLVABLE_TEST(CanCreateDefault) {
		// Act:
		auto resolvable = typename TTraits::ResolvableType();

		// Assert:
		EXPECT_TRUE(resolvable.isResolved());
		EXPECT_EQ(typename TTraits::UnresolvedType(), resolvable.unresolved());
		EXPECT_EQ(typename TTraits::ResolvedType(), resolvable.resolved());
		EXPECT_EQ(typename TTraits::ResolvedType(), resolvable.resolved(CreateThrowingResolverContext()));
	}

	RESOLVABLE_TEST(CanCreateFromUnresolved) {
		// Act:
		auto resolvedValue = TTraits::GenerateRandomResolvedValue();
		auto resolvable = typename TTraits::ResolvableType(test::UnresolveXor(resolvedValue));

		// Assert:
		EXPECT_FALSE(resolvable.isResolved());
		EXPECT_EQ(test::UnresolveXor(resolvedValue), resolvable.unresolved());
		EXPECT_THROW(resolvable.resolved(), catapult_invalid_argument);
		EXPECT_EQ(resolvedValue, resolvable.resolved(test::CreateResolverContextXor()));
	}

	RESOLVABLE_TEST(CanCreateFromResolved) {
		// Act:
		auto resolvedValue = TTraits::GenerateRandomResolvedValue();
		auto resolvable = typename TTraits::ResolvableType(resolvedValue);

		// Assert:
		EXPECT_TRUE(resolvable.isResolved());
		EXPECT_EQ(TTraits::Unresolve(resolvedValue), resolvable.unresolved());
		EXPECT_EQ(resolvedValue, resolvable.resolved());
		EXPECT_EQ(resolvedValue, resolvable.resolved(CreateThrowingResolverContext()));
	}

	// endregion
}}
