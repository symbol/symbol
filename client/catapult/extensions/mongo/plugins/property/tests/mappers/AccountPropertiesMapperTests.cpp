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

#include "src/mappers/AccountPropertiesMapper.h"
#include "plugins/txes/property/src/state/AccountProperty.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/AccountPropertiesMapperTestUtils.h"
#include "tests/test/AccountPropertiesTestUtils.h"
#include "tests/TestHarness.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <set>

namespace catapult { namespace mongo { namespace plugins {

#define TEST_CLASS AccountPropertiesMapperTests

	namespace {
		void AssertDbView(
				const state::AccountProperties& accountProperties,
				const bsoncxx::document::view& view,
				size_t numExpectedPropertyValues) {
			// Assert:
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto accountPropertiesView = view["accountProperties"].get_document().view();
			EXPECT_EQ(2u, test::GetFieldCount(accountPropertiesView));

			size_t numPropertyValues = 0u;
			for (const auto& pair : accountProperties)
				numPropertyValues += pair.second.values().size();

			EXPECT_EQ(numExpectedPropertyValues, numPropertyValues);

			test::AssertEqualAccountPropertiesData(accountProperties, accountPropertiesView);
		}
	}

	// region ToDbModel

	namespace {
		void AssertCanMapAccountProperties(const std::vector<size_t>& valueSizes, size_t numExpectedPropertyValues) {
			// Arrange:
			auto accountProperties = test::CreateAccountProperties(state::OperationType::Allow, valueSizes);

			// Act:
			auto document = ToDbModel(accountProperties);
			auto view = document.view();

			// Assert:
			AssertDbView(accountProperties, view, numExpectedPropertyValues);
		}
	}

	TEST(TEST_CLASS, CanMapAccountPropertiesWithNoValues_ModelToDbModel) {
		// Assert:
		AssertCanMapAccountProperties({ 0, 0, 0 }, 0);
	}

	TEST(TEST_CLASS, CanMapAccountPropertiesWithSingleNonEmptyProperty_ModelToDbModel) {
		// Assert:
		AssertCanMapAccountProperties({ 5, 0, 0 }, 5);
		AssertCanMapAccountProperties({ 0, 5, 0 }, 5);
		AssertCanMapAccountProperties({ 0, 0, 5 }, 5);
	}

	TEST(TEST_CLASS, CanMapAccountPropertiesWithMultipleNonEmptyProperties_ModelToDbModel) {
		// Assert:
		AssertCanMapAccountProperties({ 5, 7, 11 }, 23);
	}

	// endregion

	// region ToAccountProperties

	namespace {
		bsoncxx::document::value CreateDbAccountProperties(const std::vector<size_t>& valueSizes) {
			auto accountProperties = test::CreateAccountProperties(state::OperationType::Allow, valueSizes);
			return ToDbModel(accountProperties);
		}

		void AssertCanMapDbAccountProperties(const std::vector<size_t>& numValues, size_t numExpectedPropertyValues) {
			// Arrange:
			auto dbAccountProperties = CreateDbAccountProperties(numValues);

			// Act:
			auto accountProperties = ToAccountProperties(dbAccountProperties);
			auto view = dbAccountProperties.view();

			// Assert:
			AssertDbView(accountProperties, view, numExpectedPropertyValues);
		}
	}

	TEST(TEST_CLASS, CanMapDbAccountPropertiesWithNoValues_DbModelToModel) {
		// Assert:
		AssertCanMapDbAccountProperties({ 0, 0, 0 }, 0);
	}

	TEST(TEST_CLASS, CanMapDbAccountPropertiesWithSingleNonEmptyProperty_DbModelToModel) {
		// Assert:
		AssertCanMapDbAccountProperties({ 5, 0, 0 }, 5);
		AssertCanMapDbAccountProperties({ 0, 5, 0 }, 5);
		AssertCanMapDbAccountProperties({ 0, 0, 5 }, 5);
	}

	TEST(TEST_CLASS, CanMapDbAccountPropertiesWithMultipleNonEmptyProperties_DbModelToModel) {
		// Assert:
		AssertCanMapDbAccountProperties({ 5, 7, 11 }, 23);
	}

	// endregion
}}}
