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

#include "src/mappers/NamespaceDescriptorMapper.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "plugins/txes/namespace/src/state/RootNamespace.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "plugins/txes/namespace/tests/test/NamespaceTestUtils.h"
#include "tests/test/NamespaceMapperTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS NamespaceDescriptorMapperTests

	namespace {
		using Path = state::Namespace::Path;

		struct NoAliasTraits {
			static auto CreateAlias() {
				return state::NamespaceAlias();
			}
		};

		struct MosaicAliasTraits {
			static auto CreateAlias() {
				return state::NamespaceAlias(test::GenerateRandomValue<MosaicId>());
			}
		};

		struct AddressAliasTraits {
			static auto CreateAlias() {
				return state::NamespaceAlias(test::GenerateRandomByteArray<Address>());
			}
		};
	}

#define ALIAS_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NoAliasTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MosaicAlias) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicAliasTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_AddressAlias) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressAliasTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region ToDbModel

	namespace {
		enum class NamespaceStatus { Active, Inactive };

		NamespaceDescriptor CreateNamespaceDescriptor(uint8_t depth, NamespaceStatus status, const state::NamespaceAlias& alias) {
			Path path;
			for (auto i = 0u; i < depth; ++i)
				path.push_back(test::GenerateRandomValue<NamespaceId>());

			auto owner = test::CreateRandomOwner();
			auto pRoot = std::make_shared<state::RootNamespace>(path[0], owner, state::NamespaceLifetime(Height(123), Height(234)));
			return NamespaceDescriptor(path, alias, pRoot, test::GenerateRandomAddress(), 321, NamespaceStatus::Active == status);
		}

		void AssertCanMapNamespaceDescriptor(uint8_t depth, NamespaceStatus status, const state::NamespaceAlias& alias) {
			// Arrange:
			auto descriptor = CreateNamespaceDescriptor(depth, status, alias);

			// Act:
			auto document = ToDbModel(descriptor);
			auto documentView = document.view();

			// Assert:
			EXPECT_EQ(2u, test::GetFieldCount(documentView));

			auto metaView = documentView["meta"].get_document().view();
			EXPECT_EQ(2u, test::GetFieldCount(metaView));
			test::AssertEqualNamespaceMetadata(descriptor, metaView);

			auto namespaceView = documentView["namespace"].get_document().view();
			EXPECT_EQ(7u + depth, test::GetFieldCount(namespaceView));
			test::AssertEqualNamespaceData(descriptor, namespaceView);
		}
	}

	ALIAS_TRAITS_BASED_TEST(CanMapNamespaceDescriptor_ModelToDbModel_Depth1) {
		AssertCanMapNamespaceDescriptor(1, NamespaceStatus::Inactive, TTraits::CreateAlias());
		AssertCanMapNamespaceDescriptor(1, NamespaceStatus::Active, TTraits::CreateAlias());
	}

	ALIAS_TRAITS_BASED_TEST(CanMapNamespaceDescriptor_ModelToDbModel_Depth2) {
		AssertCanMapNamespaceDescriptor(2, NamespaceStatus::Inactive, TTraits::CreateAlias());
		AssertCanMapNamespaceDescriptor(2, NamespaceStatus::Active, TTraits::CreateAlias());
	}

	ALIAS_TRAITS_BASED_TEST(CanMapNamespaceDescriptor_ModelToDbModel_Depth3) {
		AssertCanMapNamespaceDescriptor(3, NamespaceStatus::Inactive, TTraits::CreateAlias());
		AssertCanMapNamespaceDescriptor(3, NamespaceStatus::Active, TTraits::CreateAlias());
	}

	// endregion
}}}
