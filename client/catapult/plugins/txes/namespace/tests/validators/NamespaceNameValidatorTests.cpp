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
#include "src/model/NamespaceIdGenerator.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NamespaceNameValidatorTests
#define RESERVED_TEST_CLASS NamespaceReservedNameValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(NamespaceName, 0)
	DEFINE_COMMON_VALIDATOR_TESTS(NamespaceReservedName, {})

	namespace {
		model::NamespaceNameNotification CreateNamespaceNameNotification(uint8_t nameSize, const uint8_t* pName) {
			auto notification = model::NamespaceNameNotification(NamespaceId(), NamespaceId(777), nameSize, pName);
			notification.NamespaceId = model::GenerateNamespaceId(NamespaceId(777), reinterpret_cast<const char*>(pName));
			return notification;
		}
	}

	// region name size

	namespace {
		void AssertSizeValidationResult(ValidationResult expectedResult, uint8_t nameSize, uint8_t maxNameSize) {
			// Arrange:
			auto pValidator = CreateNamespaceNameValidator(maxNameSize);
			auto name = std::string(nameSize, 'a');
			auto notification = CreateNamespaceNameNotification(nameSize, reinterpret_cast<const uint8_t*>(name.data()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "nameSize " << static_cast<uint16_t>(nameSize)
					<< ", maxNameSize " << static_cast<uint16_t>(maxNameSize);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNamespaceWithNameSizeLessThanMax) {
		AssertSizeValidationResult(ValidationResult::Success, 100, 123);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNamespaceWithNameSizeEqualToMax) {
		AssertSizeValidationResult(ValidationResult::Success, 123, 123);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNamespaceWithNameSizeGreaterThanMax) {
		AssertSizeValidationResult(Failure_Namespace_Invalid_Name, 124, 123);
		AssertSizeValidationResult(Failure_Namespace_Invalid_Name, 200, 123);
	}

	TEST(TEST_CLASS, FailureWhenValidatingEmptyNamespaceName) {
		AssertSizeValidationResult(Failure_Namespace_Invalid_Name, 0, 123);
	}

	// endregion

	// region name characters

	namespace {
		void AssertNameValidationResult(ValidationResult expectedResult, const std::string& name) {
			// Arrange:
			auto pValidator = CreateNamespaceNameValidator(static_cast<uint8_t>(name.size()));
			auto notification = CreateNamespaceNameNotification(
					static_cast<uint8_t>(name.size()),
					reinterpret_cast<const uint8_t*>(name.data()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "namespace with name " << name;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingValidNamespaceNames) {
		for (const auto& name : { "a", "be", "cat", "doom", "al-ce", "al_ce", "alice-", "alice_" })
			AssertNameValidationResult(ValidationResult::Success, name);
	}

	TEST(TEST_CLASS, FailureWhenValidatingInvalidNamespaceNames) {
		for (const auto& name : { "-alice", "_alice", "al.ce", "alIce", "al ce", "al@ce", "al#ce", "!@#$%" })
			AssertNameValidationResult(Failure_Namespace_Invalid_Name, name);
	}

	// endregion

	// region name and id consistency

	TEST(TEST_CLASS, SuccessWhenValidatingNamespaceWithMatchingNameAndId) {
		// Arrange: note that CreateNamespaceNameNotification creates proper id
		auto pValidator = CreateNamespaceNameValidator(100);
		auto name = std::string(10, 'a');
		auto notification = CreateNamespaceNameNotification(
				static_cast<uint8_t>(name.size()),
				reinterpret_cast<const uint8_t*>(name.data()));

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	TEST(TEST_CLASS, FailureWhenValidatingNamespaceWithMismatchedNameAndId) {
		// Arrange: corrupt the id
		auto pValidator = CreateNamespaceNameValidator(100);
		auto name = std::string(10, 'a');
		auto notification = CreateNamespaceNameNotification(
				static_cast<uint8_t>(name.size()),
				reinterpret_cast<const uint8_t*>(name.data()));
		notification.NamespaceId = notification.NamespaceId + NamespaceId(1);

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(Failure_Namespace_Name_Id_Mismatch, result);
	}

	// endregion

	// region reserved root names

	namespace {
		constexpr auto Valid_Names = std::initializer_list<const char*>{ "fo", "foo123", "bar", "bazz", "qux" };
		constexpr auto Reserved_Names = std::initializer_list<const char*>{ "foo", "foobar" };

		model::NamespaceNameNotification CreateRootNamespaceNotification(const std::string& name) {
			auto nameSize = static_cast<uint8_t>(name.size());
			const auto* pName = reinterpret_cast<const uint8_t*>(name.data());
			auto notification = model::NamespaceNameNotification(NamespaceId(), Namespace_Base_Id, nameSize, pName);
			notification.NamespaceId = model::GenerateNamespaceId(Namespace_Base_Id, name);
			return notification;
		}

		model::NamespaceNameNotification CreateChildNamespaceNotification(const std::string& parentName) {
			const auto* pChildName = reinterpret_cast<const uint8_t*>("alice");
			auto notification = model::NamespaceNameNotification(NamespaceId(), NamespaceId(), 5, pChildName);
			notification.ParentId = model::GenerateNamespaceId(Namespace_Base_Id, parentName);
			notification.NamespaceId = model::GenerateNamespaceId(notification.ParentId, reinterpret_cast<const char*>(pChildName));
			return notification;
		}

		void AssertReservedRootNamesValidationResult(
				ValidationResult expectedResult,
				const std::string& name,
				const std::function<model::NamespaceNameNotification(const std::string&)>& createNotification,
				Height notificationHeight = Height(2)) {
			// Arrange:
			auto reservedNames = std::unordered_set<std::string>(Reserved_Names.begin(), Reserved_Names.end());
			auto pValidator = CreateNamespaceReservedNameValidator(reservedNames);
			auto notification = createNotification(name);

			// Act:
			auto cache = test::CreateEmptyCatapultCache();
			auto result = test::ValidateNotification(*pValidator, notification, cache, notificationHeight);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "namespace with name " << name;
		}
	}

	TEST(RESERVED_TEST_CLASS, SuccessWhenValidatingAllowedRootNamespaceNames) {
		for (const auto& name : Valid_Names)
			AssertReservedRootNamesValidationResult(ValidationResult::Success, name, CreateRootNamespaceNotification);
	}

	TEST(RESERVED_TEST_CLASS, FailureWhenValidatingReservedRootNamespaceNames) {
		for (const auto& name : Reserved_Names)
			AssertReservedRootNamesValidationResult(Failure_Namespace_Root_Name_Reserved, name, CreateRootNamespaceNotification);
	}

	TEST(RESERVED_TEST_CLASS, SuccessWhenValidatingReservedRootNamespaceNamesUsedByNemesis) {
		for (const auto& name : Reserved_Names)
			AssertReservedRootNamesValidationResult(ValidationResult::Success, name, CreateRootNamespaceNotification, Height(1));
	}

	TEST(RESERVED_TEST_CLASS, SuccessWhenValidatingChildNamespaceWithAllowedParentNamespaceNames) {
		for (const auto& name : Valid_Names)
			AssertReservedRootNamesValidationResult(ValidationResult::Success, name, CreateChildNamespaceNotification);
	}

	TEST(RESERVED_TEST_CLASS, FailureWhenValidatingChildNamespaceWithReservedParentNamespaceNames) {
		for (const auto& name : Reserved_Names)
			AssertReservedRootNamesValidationResult(Failure_Namespace_Root_Name_Reserved, name, CreateChildNamespaceNotification);
	}

	TEST(RESERVED_TEST_CLASS, SuccessWhenValidatingChildNamespaceWithReservedParentNamespaceNamesUsedByNemesis) {
		for (const auto& name : Reserved_Names)
			AssertReservedRootNamesValidationResult(ValidationResult::Success, name, CreateChildNamespaceNotification, Height(1));
	}

	// endregion
}}
