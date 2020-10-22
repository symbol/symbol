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
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region (single) attachment pointer

	/// Asserts that attachment data pointer is inaccessible when entity has size less than minimum entity size.
	template<typename TEntityTraits, typename TAccessor>
	void AssertAttachmentPointerIsInaccessibleWhenSizeIsLessThanEntitySize() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(0);
		pEntity->Size = sizeof(typename decltype(pEntity)::element_type) - 1;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer(accessor));
	}

	/// Asserts that attachment data pointer is inaccessible when entity has no attachments.
	template<typename TEntityTraits, typename TAccessor>
	void AssertAttachmentPointerIsInaccessibleWhenNoAttachmentsArePresent() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(0);
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer(accessor));
	}

	/// Asserts that attachment data pointer is inaccessible when entity size is too small.
	template<typename TEntityTraits, typename TAccessor>
	void AssertAttachmentPointerIsInaccessibleWhenSizeIsLessThanCalculatedSize() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(3);
		--pEntity->Size;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer(accessor));
	}

	/// Asserts that attachment data pointer is inaccessible when entity size is too large.
	template<typename TEntityTraits, typename TAccessor>
	void AssertAttachmentPointerIsInaccessibleWhenSizeIsGreaterThanCalculatedSize() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(3);
		++pEntity->Size;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer(accessor));
	}

	struct OffsetAccessor {
	public:
		template<typename TTraits>
		static uint32_t Get() {
			return Get<TTraits>(Accessor<TTraits>());
		}

	private:
		enum class FeatureType { Unsupported, Supported };
		using UnsupportedFeatureFlag = std::integral_constant<FeatureType, FeatureType::Unsupported>;
		using SupportedFeatureFlag = std::integral_constant<FeatureType, FeatureType::Supported>;

		template<typename T, typename = void>
		struct Accessor : public UnsupportedFeatureFlag {};

		template<typename T>
		struct Accessor<
				T,
				utils::traits::is_type_expression_t<decltype(T::Offset)>>
				: public SupportedFeatureFlag
		{};

	private:
		template<typename TTraits>
		static uint32_t Get(UnsupportedFeatureFlag) {
			return 0;
		}

		template<typename TTraits>
		static uint32_t Get(SupportedFeatureFlag) {
			return TTraits::Offset;
		}
	};

	/// Asserts that attachment data pointer is accessible when attachments are present and size is correct.
	template<typename TEntityTraits, typename TAccessor>
	void AssertAttachmentPointerIsValidWhenAttachmentsArePresent() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(3);
		auto offset = OffsetAccessor::Get<TEntityTraits>();
		auto pEntityEnd = test::AsVoidPointer(reinterpret_cast<const uint8_t*>(pEntity.get() + 1) + offset);
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_EQ(pEntityEnd, TEntityTraits::GetAttachmentPointer(accessor));
	}

	// endregion

	// region (dual) attachment pointers

	/// Asserts that data pointers are inaccessible when entity has size less than minimum entity size.
	template<typename TEntityTraits, typename TAccessor>
	void AssertDataPointersAreInaccessibleWhenSizeIsLessThanEntitySize() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(111, 5);
		pEntity->Size = sizeof(typename decltype(pEntity)::element_type) - 1;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that data pointers are inaccessible when entity has no attachments.
	template<typename TEntityTraits, typename TAccessor>
	void AssertDataPointersAreInaccessibleWhenNoDataArePresent() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(0, 0);
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that data pointers are inaccessible when entity size is too small and contains only first attachment.
	template<typename TEntityTraits, typename TAccessor>
	void AssertDataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithFirstData() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(111, 0);
		--pEntity->Size;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that data pointers are inaccessible when entity size is too small and contains only second attachment.
	template<typename TEntityTraits, typename TAccessor>
	void AssertDataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithSecondData() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(0, 5);
		--pEntity->Size;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that data pointers are inaccessible when entity size is too small and contains first and second attachment.
	template<typename TEntityTraits, typename TAccessor>
	void AssertDataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithAllData() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(111, 5);
		--pEntity->Size;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that data pointers are inaccessible when entity size is too large and contains first and second attachment.
	template<typename TEntityTraits, typename TAccessor>
	void AssertDataPointersAreInaccessibleWhenSizeIsGreaterThanCalculatedSizeWithAllData() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(111, 5);
		++pEntity->Size;
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that the first data pointer is accessible when entity only has first attachment.
	template<typename TEntityTraits, typename TAccessor>
	void AssertFirstPointerIsValidWhenOnlyFirstDataIsPresent() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(111, 0);
		auto pEntityEnd = test::AsVoidPointer(pEntity.get() + 1);
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_EQ(pEntityEnd, TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that the second data pointer is accessible when entity only has second attachment.
	template<typename TEntityTraits, typename TAccessor>
	void AssertSecondPointerIsValidWhenOnlySecondDataIsPresent() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(0, 5);
		auto pEntityEnd = test::AsVoidPointer(pEntity.get() + 1);
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_FALSE(!!TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_EQ(pEntityEnd, TEntityTraits::GetAttachmentPointer2(accessor));
	}

	/// Asserts that the all data pointers are accessible when all attachments are present.
	template<typename TEntityTraits, typename TAccessor>
	void AssertAllPointersAreValidWhenAllDataArePresent() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(111, 5);
		auto pEntityEnd = test::AsVoidPointer(pEntity.get() + 1);
		auto pEntity2End = test::AsVoidPointer(reinterpret_cast<const uint8_t*>(pEntityEnd) + TEntityTraits::GetAttachment1Size(111));
		auto& accessor = TAccessor::Get(*pEntity);

		// Act + Assert:
		EXPECT_EQ(pEntityEnd, TEntityTraits::GetAttachmentPointer1(accessor));
		EXPECT_EQ(pEntity2End, TEntityTraits::GetAttachmentPointer2(accessor));
	}

	// endregion

	/// Traits for accessing an entity as const.
	struct ConstAccessor {
		/// Gets a const reference.
		template<typename T>
		static const T& Get(const T& entity) {
			return entity;
		}
	};

	/// Traits for accessing an entity as non-const.
	struct NonConstAccessor {
		/// Gets a non-const reference.
		template<typename T, typename X = std::enable_if_t<!std::is_const_v<T>>>
		static T& Get(T& entity) {
			return entity;
		}
	};
}}

#define DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_Const) { test::Assert##TEST_NAME<TEST_TRAITS, test::ConstAccessor>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { test::Assert##TEST_NAME<TEST_TRAITS, test::NonConstAccessor>(); } \

/// Adds all attachment pointer tests to the specified test class (\a TEST_CLASS) using \a TEST_TRAITS.
/// \note These tests only support entities with a single pointer.
#define DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, TEST_TRAITS) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenSizeIsLessThanEntitySize) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenNoAttachmentsArePresent) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenSizeIsLessThanCalculatedSize) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenSizeIsGreaterThanCalculatedSize) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsValidWhenAttachmentsArePresent)

/// Adds all attachment pointer tests to the specified test class (\a TEST_CLASS) using \a TEST_TRAITS.
/// \note These tests only support entities with two pointers.
#define DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(TEST_CLASS, TEST_TRAITS) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanEntitySize) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenNoDataArePresent) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithFirstData) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithSecondData) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithAllData) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsGreaterThanCalculatedSizeWithAllData) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, FirstPointerIsValidWhenOnlyFirstDataIsPresent) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, SecondPointerIsValidWhenOnlySecondDataIsPresent) \
	DEFINE_ATTACHMENT_ACCESS_TESTS(TEST_CLASS, TEST_TRAITS, AllPointersAreValidWhenAllDataArePresent)
