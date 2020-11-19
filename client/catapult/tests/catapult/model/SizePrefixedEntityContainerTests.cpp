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

#include "catapult/model/SizePrefixedEntityContainer.h"
#include "catapult/utils/IntegerMath.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/SizePrefixedEntityContainerTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS SizePrefixedEntityContainerTests

	namespace {
		constexpr uint32_t Expanded_Header_Size = 16;

		// region Block (implicit size) emulation

#pragma pack(push, 1)

		// emulates BlockHeader
		struct ContainerHeader : public SizePrefixedEntity {
			uint32_t Tag;
		};

		// emulates Transaction (intentionally has different size than ContainerHeader)
		struct ContainerComponent : public SizePrefixedEntity {
			uint16_t Id;
		};

#pragma pack(pop)

		struct ExpandedHeaderProperties {
			template<typename TContainer>
			static size_t HeaderSize(const TContainer&) {
				return Expanded_Header_Size;
			}
		};

		// emulates Block
		struct Container : public TransactionContainer<ContainerHeader, ContainerComponent> {};
		struct ExpandedHeaderContainer : public TransactionContainer<ContainerHeader, ContainerComponent, ExpandedHeaderProperties> {};

		size_t GetTransactionPayloadSize(const ContainerHeader& header) {
			return header.Size - (header.Tag ? Expanded_Header_Size : sizeof(ContainerHeader));
		}

		// endregion

		// region AggregateTransaction (explicit size) emulation

#pragma pack(push, 1)

		// emulates AggregateTransactionHeader
		struct ContainerWithPayloadSizeHeader : public SizePrefixedEntity {
			uint32_t Tag;
			uint32_t PayloadSize;
		};

#pragma pack(pop)

		// emulates AggregateTransaction
		struct ContainerWithPayloadSize : public TransactionContainer<ContainerWithPayloadSizeHeader, ContainerComponent> {};
		struct ExpandedHeaderContainerWithPayloadSize
				: public TransactionContainer<ContainerWithPayloadSizeHeader, ContainerComponent, ExpandedHeaderProperties>
		{};

		size_t GetTransactionPayloadSize(const ContainerWithPayloadSizeHeader& header) {
			return header.PayloadSize;
		}

		// endregion

		// region HasExpandedHeader / CalculateHeaderSize

		template<typename TContainer>
		bool HasExpandedHeader() {
			return true;
		}

		template<>
		bool HasExpandedHeader<Container>() {
			return false;
		}

		template<typename TContainer>
		uint32_t CalculateHeaderSize() {
			return Expanded_Header_Size;
		}

		template<>
		uint32_t CalculateHeaderSize<Container>() {
			return sizeof(Container);
		}

		template<>
		uint32_t CalculateHeaderSize<ContainerWithPayloadSize>() {
			return sizeof(ContainerWithPayloadSize);
		}

		// endregion

		// region CreateContainer

		uint32_t CalculateContainerSize(uint32_t headerSize, std::initializer_list<uint32_t> attachmentExtraSizes) {
			uint32_t size = headerSize;
			uint32_t lastPaddingSize = 0;
			for (auto attachmentExtraSize : attachmentExtraSizes) {
				uint32_t attachmentSize = SizeOf32<ContainerComponent>() + attachmentExtraSize;
				lastPaddingSize = utils::GetPaddingSize(attachmentSize, 8);

				size += attachmentSize + lastPaddingSize;
			}

			return size - lastPaddingSize; // last element shouldn't be padded
		}

		template<typename TContainer>
		std::unique_ptr<TContainer> CreateContainer(std::initializer_list<uint32_t> attachmentExtraSizes) {
			auto headerSize = CalculateHeaderSize<TContainer>();
			auto size = CalculateContainerSize(headerSize, attachmentExtraSizes);
			auto pContainer = utils::MakeUniqueWithSize<TContainer>(size);
			pContainer->Size = size;
			pContainer->Tag = HasExpandedHeader<TContainer>() ? 1 : 0;

			auto* pData = reinterpret_cast<uint8_t*>(pContainer.get()) + headerSize;
			for (auto attachmentExtraSize : attachmentExtraSizes) {
				uint32_t attachmentSize = SizeOf32<ContainerComponent>() + attachmentExtraSize;
				reinterpret_cast<ContainerComponent*>(pData)->Size = attachmentSize;
				pData += attachmentSize + utils::GetPaddingSize(attachmentSize, 8);
			}

			return pContainer;
		}

		// endregion
	}

	// region traits

#define IMPLICIT_DATA_POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::ConstTraitsT<Container>>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::NonConstTraitsT<Container>>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Const_ExpandedHeader) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::ConstTraitsT<ExpandedHeaderContainer>>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_NonConst_ExpandedHeader) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::NonConstTraitsT<ExpandedHeaderContainer>>(); \
	} \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

#define EXPLICIT_DATA_POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::ConstTraitsT<ContainerWithPayloadSize>>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::NonConstTraitsT<ContainerWithPayloadSize>>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_Const_ExpandedHeader) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::ConstTraitsT<ExpandedHeaderContainerWithPayloadSize>>(); \
	} \
	TEST(TEST_CLASS, TEST_NAME##_NonConst_ExpandedHeader) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::NonConstTraitsT<ExpandedHeaderContainerWithPayloadSize>>(); \
	} \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region implicit size

	IMPLICIT_DATA_POINTER_TEST(TransactionsAreInaccessibleWhenContainerHasNoTransactions_ImplicitSize) {
		// Arrange:
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({});
		auto& accessor = TTraits::GetAccessor(*pContainer);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.TransactionsPtr());
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.Transactions()));
	}

	IMPLICIT_DATA_POINTER_TEST(TransactionsAreAccessibleWhenContainerHasTransactionsWithSizesEqualToPayloadSize_ImplicitSize) {
		// Arrange:
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({ 100, 50, 75 });
		auto& accessor = TTraits::GetAccessor(*pContainer);

		auto headerSize = CalculateHeaderSize<ContainerType>();
		const auto* pContainerEnd = test::AsVoidPointer(reinterpret_cast<const uint8_t*>(pContainer.get()) + headerSize);

		// Act + Assert:
		EXPECT_EQ(pContainerEnd, accessor.TransactionsPtr());
		EXPECT_EQ(3u, test::CountContainerEntities(accessor.Transactions()));
	}

	IMPLICIT_DATA_POINTER_TEST(TransactionsAreInaccessibleWhenReportedSizeIsLessThanContainerHeaderSize_ImplicitSize) {
		// Arrange:
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({ 100, 50, 75 });
		pContainer->Size = CalculateHeaderSize<ContainerType>() - 1;
		auto& accessor = TTraits::GetAccessor(*pContainer);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.TransactionsPtr());
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.Transactions()));
	}

	IMPLICIT_DATA_POINTER_TEST(TransactionsArePartiallyAccessibleWhenContainerHasTransactionsWithSizesNotEqualToPayloadSize_ImplicitSize) {
		// Arrange:
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({ 100, 50, 75 });
		--pContainer->Size;
		auto& accessor = TTraits::GetAccessor(*pContainer);

		// Act + Assert:
		EXPECT_TRUE(!!accessor.TransactionsPtr());
		EXPECT_EQ(2u, test::CountContainerEntities(accessor.Transactions(EntityContainerErrorPolicy::Suppress)));
		EXPECT_THROW(test::CountContainerEntities(accessor.Transactions()), catapult_runtime_error);
	}

	// endregion

	// region explicit size

	EXPLICIT_DATA_POINTER_TEST(TransactionsAreInaccessibleWhenContainerHasNoTransactions_ExplicitSize) {
		// Arrange:
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({ 100, 50, 75 });
		pContainer->PayloadSize = 0;
		auto& accessor = TTraits::GetAccessor(*pContainer);

		// Sanity:
		EXPECT_LT(CalculateHeaderSize<ContainerType>(), pContainer->Size);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.TransactionsPtr());
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.Transactions()));
	}

	EXPLICIT_DATA_POINTER_TEST(TransactionsAreAccessibleWhenContainerHasTransactionsWithSizesEqualToPayloadSize_ExplicitSize) {
		// Arrange: padding is applied to 2-byte header and data size
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({ 100, 50, 75 });
		pContainer->PayloadSize = 3 * sizeof(ContainerComponent) + 100 + 2 + 50 + 4 + 75;
		auto& accessor = TTraits::GetAccessor(*pContainer);

		auto headerSize = CalculateHeaderSize<ContainerType>();
		const auto* pContainerEnd = test::AsVoidPointer(reinterpret_cast<const uint8_t*>(pContainer.get()) + headerSize);

		// Act + Assert:
		EXPECT_EQ(pContainerEnd, accessor.TransactionsPtr());
		EXPECT_EQ(3u, test::CountContainerEntities(accessor.Transactions()));
	}

	EXPLICIT_DATA_POINTER_TEST(TransactionsAreInaccessibleWhenReportedSizeIsLessThanContainerHeaderSize_ExplicitSize) {
		// Arrange: padding is applied to 2-byte header and data size
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({ 100, 50, 75 });
		pContainer->PayloadSize = 3 * sizeof(ContainerComponent) + 100 + 2 + 50 + 4 + 75;
		pContainer->Size = CalculateHeaderSize<ContainerType>() - 1;
		auto& accessor = TTraits::GetAccessor(*pContainer);

		// Act + Assert:
		EXPECT_FALSE(!!accessor.TransactionsPtr());
		EXPECT_EQ(0u, test::CountContainerEntities(accessor.Transactions()));
	}

	EXPLICIT_DATA_POINTER_TEST(TransactionsArePartiallyAccessibleWhenContainerHasTransactionsWithSizesNotEqualToPayloadSize_ExplicitSize) {
		// Arrange: padding is applied to 2-byte header and data size
		using ContainerType = std::remove_const_t<typename TTraits::SourceType>;
		auto pContainer = CreateContainer<ContainerType>({ 100, 50, 75 });
		pContainer->PayloadSize = 3 * sizeof(ContainerComponent) + 100 + 2 + 50 + 4 + 75;
		--pContainer->PayloadSize;
		auto& accessor = TTraits::GetAccessor(*pContainer);

		// Act + Assert:
		EXPECT_TRUE(!!accessor.TransactionsPtr());
		EXPECT_EQ(2u, test::CountContainerEntities(accessor.Transactions(EntityContainerErrorPolicy::Suppress)));
		EXPECT_THROW(test::CountContainerEntities(accessor.Transactions()), catapult_runtime_error);
	}

	// endregion
}}
