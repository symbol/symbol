#pragma once
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region IsSizeValid

	/// Asserts that entity with correct size is valid.
	template<typename TEntityTraits>
	void AssertSizeIsValidWhenEntitySizeIsCorrect() {
		// Arrange:
		auto entity = TEntityTraits::Create();
		entity.Size = static_cast<uint32_t>(CalculateRealSize(entity));

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(entity));
	}

	/// Asserts that entity with size too small is invalid.
	template<typename TEntityTraits>
	void AssertSizeIsInvalidWhenEntitySizeIsTooSmall() {
		// Arrange:
		auto entity = TEntityTraits::Create();
		entity.Size = static_cast<uint32_t>(CalculateRealSize(entity)) - 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(entity));
	}

	/// Asserts that entity with size too large is invalid.
	template<typename TEntityTraits>
	void AssertSizeIsInvalidWhenEntitySizeIsTooLarge() {
		// Arrange:
		auto entity = TEntityTraits::Create();
		entity.Size = static_cast<uint32_t>(CalculateRealSize(entity)) + 1;

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(entity));
	}

	// endregion

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

	/// Asserts that attachment data pointer is accessible when attachments are present and size is correct.
	template<typename TEntityTraits, typename TAccessor>
	void AssertAttachmentPointerIsValidWhenAttachmentsArePresent() {
		// Arrange:
		auto pEntity = TEntityTraits::GenerateEntityWithAttachments(3);
		auto pEntityEnd = test::AsVoidPointer(pEntity.get() + 1);
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
		template<typename T, typename X = typename std::enable_if<!std::is_const<T>::value>::type>
		static T& Get(T& entity) {
			return entity;
		}
	};
}}

#define DEFINE_IS_SIZE_VALID_TEST(TEST_CLASS, TEST_TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::Assert##TEST_NAME<TEST_TRAITS>(); } \

/// Adds all IsSizeValid tests to the specified test class (\a TEST_CLASS) using \a TEST_TRAITS.
#define DEFINE_IS_SIZE_VALID_TESTS(TEST_CLASS, TEST_TRAITS) \
	DEFINE_IS_SIZE_VALID_TEST(TEST_CLASS, TEST_TRAITS, SizeIsValidWhenEntitySizeIsCorrect) \
	DEFINE_IS_SIZE_VALID_TEST(TEST_CLASS, TEST_TRAITS, SizeIsInvalidWhenEntitySizeIsTooSmall) \
	DEFINE_IS_SIZE_VALID_TEST(TEST_CLASS, TEST_TRAITS, SizeIsInvalidWhenEntitySizeIsTooLarge)

#define DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME##_Const) { test::Assert##TEST_NAME<TEST_TRAITS, test::ConstAccessor>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { test::Assert##TEST_NAME<TEST_TRAITS, test::NonConstAccessor>(); } \

/// Adds all attachment pointer tests to the specified test class (\a TEST_CLASS) using \a TEST_TRAITS.
/// \note These tests only support entities with a single pointer.
#define DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, TEST_TRAITS) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenSizeIsLessThanEntitySize) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenNoAttachmentsArePresent) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenSizeIsLessThanCalculatedSize) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsInaccessibleWhenSizeIsGreaterThanCalculatedSize) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, AttachmentPointerIsValidWhenAttachmentsArePresent)

/// Adds all attachment pointer tests to the specified test class (\a TEST_CLASS) using \a TEST_TRAITS.
/// \note These tests only support entities with two pointers.
#define DEFINE_DUAL_ATTACHMENT_POINTER_TESTS(TEST_CLASS, TEST_TRAITS) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanEntitySize) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenNoDataArePresent) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithFirstData) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithSecondData) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsLessThanCalculatedSizeWithAllData) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, DataPointersAreInaccessibleWhenSizeIsGreaterThanCalculatedSizeWithAllData) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, FirstPointerIsValidWhenOnlyFirstDataIsPresent) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, SecondPointerIsValidWhenOnlySecondDataIsPresent) \
	DEFINE_ATTACHMENT_POINTER_TEST(TEST_CLASS, TEST_TRAITS, AllPointersAreValidWhenAllDataArePresent)
