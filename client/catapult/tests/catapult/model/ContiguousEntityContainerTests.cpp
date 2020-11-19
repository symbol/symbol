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

#include "catapult/model/ContiguousEntityContainer.h"
#include "tests/test/nodeps/IteratorTestTraits.h"
#include "tests/TestHarness.h"
#include <vector>

namespace catapult { namespace model {

#define TEST_CLASS ContiguousEntityContainerTests

	namespace {
		struct EntityHeader {
			uint32_t Size;
			uint32_t Value;
		};

		// notice that this function is really creating fixed size *entities* (not structures) because EntityHeader has an entity layout
		// that supports variable sizing
		std::vector<EntityHeader> CreateFixedSizedEntities(const std::vector<uint32_t>& values) {
			std::vector<EntityHeader> entities;
			for (auto value : values)
				entities.push_back({ sizeof(EntityHeader), value });

			return entities;
		}

		template<EntityContainerErrorPolicy ErrorPolicy>
		struct EntityContainerBasedTraits {
			template<typename TEntity>
			static auto MakeContainer(TEntity* pEntity, size_t count, size_t size = 0) {
				if (0 == size)
					size = count * sizeof(EntityHeader);

				return MakeContiguousEntityContainer(pEntity, size, ErrorPolicy);
			}

			template<typename TEntity>
			static auto MakeContainer(const TEntity* pEntity, size_t count, size_t size = 0) {
				if (0 == size)
					size = count * sizeof(EntityHeader);

				return MakeContiguousEntityContainer(pEntity, size, ErrorPolicy);
			}

			static constexpr bool ThrowsOnError = EntityContainerErrorPolicy::Throw == ErrorPolicy;
		};

		using EntityContainerThrowBasedTraits = EntityContainerBasedTraits<EntityContainerErrorPolicy::Throw>;
		using EntityContainerSuppressBasedTraits = EntityContainerBasedTraits<EntityContainerErrorPolicy::Suppress>;
	}

// there are four sets of tests: error policy { throw, suppress } X mutability { mutable, const }
#define TRAITS_BASED_TEST_ENTRY(TEST_NAME, DESCRIPTION, BEGIN_END_TRAITS, ERROR_TRAITS) \
	TEST(TEST_CLASS, TEST_NAME##_##DESCRIPTION) { \
		TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BEGIN_END_TRAITS, ERROR_TRAITS>(); \
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits, typename TContainerTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TRAITS_BASED_TEST_ENTRY(TEST_NAME, Throw_Mutable, test::BeginEndTraits, EntityContainerThrowBasedTraits) \
	TRAITS_BASED_TEST_ENTRY(TEST_NAME, Throw_Const, test::CBeginCEndTraits, EntityContainerThrowBasedTraits) \
	TRAITS_BASED_TEST_ENTRY(TEST_NAME, Suppress_Mutable, test::BeginEndTraits, EntityContainerSuppressBasedTraits) \
	TRAITS_BASED_TEST_ENTRY(TEST_NAME, Suppress_Const, test::CBeginCEndTraits, EntityContainerSuppressBasedTraits) \
	template<typename TTraits, typename TContainerTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

// if TContainerTraits::ThrowsOnError is set, then expect an exception; otherwise, do not
#define EXPECT_ITERATION_ERROR(STATEMENT) \
	do { \
		if (TContainerTraits::ThrowsOnError) { \
			EXPECT_THROW((STATEMENT), catapult_runtime_error); \
		} else { \
			(STATEMENT); \
		} \
	} while (false)

	TRAITS_BASED_TEST(CanIterateOverZeroEntities) {
		// Arrange:
		auto container = TContainerTraits::template MakeContainer<EntityHeader>(nullptr, 0);

		// Act + Assert:
		EXPECT_EQ(TTraits::begin(container), TTraits::end(container));
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(CanIterateOverSingleEntityWithPostfixOperator) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container = TContainerTraits::MakeContainer(&entities[0], 1);

		// Act + Assert:
		auto iter = TTraits::begin(container);
		EXPECT_EQ(17u, (*iter++).Value);
		EXPECT_EQ(iter, TTraits::end(container));
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(CanIterateOverSingleEntityWithPrefixOperator) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container = TContainerTraits::MakeContainer(&entities[0], 1);

		// Act + Assert:
		auto iter = TTraits::begin(container);
		EXPECT_EQ(17u, iter->Value);
		++iter;
		EXPECT_EQ(iter, TTraits::end(container));
		EXPECT_FALSE(container.hasError());
	}

	namespace {
		template<typename TTraits, typename TContainerTraits, typename TEntity>
		void AssertCanIterateOverMultipleEntitiesWithPostfixOperator() {
			// Arrange:
			auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
			auto container = TContainerTraits::template MakeContainer<TEntity>(&entities[0], 3);

			// Act + Assert:
			auto iter = TTraits::begin(container);
			EXPECT_EQ(17u, (*iter++).Value);
			EXPECT_EQ(25u, (*iter++).Value);
			EXPECT_EQ(14u, (*iter++).Value);
			EXPECT_EQ(iter, TTraits::end(container));
			EXPECT_FALSE(container.hasError());
		}

		template<typename TTraits, typename TContainerTraits, typename TEntity>
		void AssertCanIterateOverMultipleEntitiesWithPrefixOperator() {
			// Arrange:
			auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
			auto container = TContainerTraits::template MakeContainer<TEntity>(&entities[0], 3);

			// Act + Assert:
			auto iter = TTraits::begin(container);
			EXPECT_EQ(17u, iter->Value);
			++iter;
			EXPECT_EQ(25u, iter->Value);
			++iter;
			EXPECT_EQ(14u, iter->Value);
			++iter;
			EXPECT_EQ(iter, TTraits::end(container));
			EXPECT_FALSE(container.hasError());
		}
	}

	TRAITS_BASED_TEST(CanIterateOverMultipleEntitiesWithPostfixOperator) {
		AssertCanIterateOverMultipleEntitiesWithPostfixOperator<TTraits, TContainerTraits, EntityHeader>();
	}

	TRAITS_BASED_TEST(CanIterateOverMultipleEntitiesWithPrefixOperator) {
		AssertCanIterateOverMultipleEntitiesWithPrefixOperator<TTraits, TContainerTraits, EntityHeader>();
	}

	TRAITS_BASED_TEST(CanIterateOverMultipleConstEntitiesWithPostfixOperator) {
		AssertCanIterateOverMultipleEntitiesWithPostfixOperator<TTraits, TContainerTraits, const EntityHeader>();
	}

	TRAITS_BASED_TEST(CanIterateOverMultipleConstEntitiesWithPrefixOperator) {
		AssertCanIterateOverMultipleEntitiesWithPrefixOperator<TTraits, TContainerTraits, const EntityHeader>();
	}

	TRAITS_BASED_TEST(CanIterateOverMultipleVariableSizedEntitiesWithPostfixOperator) {
		// Arrange: entities must be aligned on 8-byte boundaries
		std::vector<uint8_t> buffer(100);
		reinterpret_cast<EntityHeader&>(*&buffer[0]) = { 20, 17 };
		reinterpret_cast<EntityHeader&>(*&buffer[24]) = { 30, 25 };
		reinterpret_cast<EntityHeader&>(*&buffer[56]) = { 10, 14 };
		auto container = TContainerTraits::MakeContainer(reinterpret_cast<EntityHeader*>(&buffer[0]), 3, 56 + 10);

		// Act + Assert:
		auto iter = TTraits::begin(container);
		EXPECT_EQ(17u, (*iter++).Value);
		EXPECT_EQ(25u, (*iter++).Value);
		EXPECT_EQ(14u, (*iter++).Value);
		EXPECT_EQ(iter, TTraits::end(container));
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(CanIterateOverMultipleVariableSizedEntitiesWithPrefixOperator) {
		// Arrange: entities must be aligned on 8-byte boundaries
		std::vector<uint8_t> buffer(100);
		reinterpret_cast<EntityHeader&>(*&buffer[0]) = { 20, 17 };
		reinterpret_cast<EntityHeader&>(*&buffer[24]) = { 30, 25 };
		reinterpret_cast<EntityHeader&>(*&buffer[56]) = { 10, 14 };
		auto container = TContainerTraits::MakeContainer(reinterpret_cast<EntityHeader*>(&buffer[0]), 3, 56 + 10);

		// Act + Assert:
		auto iter = TTraits::begin(container);
		EXPECT_EQ(17u, iter->Value);
		++iter;
		EXPECT_EQ(25u, iter->Value);
		++iter;
		EXPECT_EQ(14u, iter->Value);
		++iter;
		EXPECT_EQ(iter, TTraits::end(container));
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(CannotIterateBeyondEndWithPostfixOperator) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container = TContainerTraits::MakeContainer(&entities[0], 3);

		// Act + Assert:
		auto iter = TTraits::end(container);
		EXPECT_THROW(iter++, catapult_out_of_range);
		EXPECT_THROW(iter++, catapult_out_of_range);
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(CannotIterateBeyondEndWithPrefixOperator) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container = TContainerTraits::MakeContainer(&entities[0], 3);

		// Act + Assert:
		auto iter = TTraits::end(container);
		EXPECT_THROW(++iter, catapult_out_of_range);
		EXPECT_THROW(++iter, catapult_out_of_range);
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(CannotDereferenceAtEnd) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container = TContainerTraits::MakeContainer(&entities[0], 3);

		// Act + Assert:
		auto iter = TTraits::end(container);
		EXPECT_THROW(*iter, catapult_out_of_range);
		EXPECT_THROW(iter.operator->(), catapult_out_of_range);
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(BeginEndIteratorsBasedOnSameContainerAreEqual) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container = TContainerTraits::MakeContainer(&entities[0], 3);

		// Act + Assert:
		EXPECT_EQ(TTraits::begin(container), TTraits::begin(container));
		EXPECT_EQ(TTraits::end(container), TTraits::end(container));
		EXPECT_NE(TTraits::begin(container), TTraits::end(container));
		EXPECT_FALSE(container.hasError());
	}

	TRAITS_BASED_TEST(BeginEndIteratorsBasedOnSameUnderlyingDataAreEqual) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container1 = TContainerTraits::MakeContainer(&entities[0], 3);
		auto container2 = TContainerTraits::MakeContainer(&entities[0], 3);

		// Act + Assert:
		EXPECT_EQ(TTraits::begin(container1), TTraits::begin(container2));
		EXPECT_EQ(TTraits::end(container1), TTraits::end(container2));
		EXPECT_FALSE(container1.hasError());
		EXPECT_FALSE(container2.hasError());
	}

	TRAITS_BASED_TEST(BeginEndIteratorsBasedOnDifferentUnderlyingDataAreNotEqual) {
		// Arrange:
		auto entities1 = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container1 = TContainerTraits::MakeContainer(&entities1[0], 3);

		auto entities2 = CreateFixedSizedEntities({ 17, 25, 14 });
		auto container2 = TContainerTraits::MakeContainer(&entities2[0], 3);

		// Act + Assert:
		EXPECT_NE(TTraits::begin(container1), TTraits::begin(container2));
		EXPECT_NE(TTraits::end(container1), TTraits::end(container2));
		EXPECT_FALSE(container1.hasError());
		EXPECT_FALSE(container2.hasError());
	}

	TRAITS_BASED_TEST(CanMutateDataUsingMutableIterator) {
		// Arrange:
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });

		// Act: increment all entity values
		auto container = TContainerTraits::MakeContainer(&entities[0], 3);
		for (auto& entity : container)
			++entity.Value;

		// Assert: all entity values were changed
		EXPECT_EQ(18u, entities[0].Value);
		EXPECT_EQ(26u, entities[1].Value);
		EXPECT_EQ(15u, entities[2].Value);
		EXPECT_FALSE(container.hasError());
	}

	namespace {
		template<typename TContainer>
		void IterateValues(TContainer& container, std::vector<uint32_t>& values) {
			for (const auto& entity : container)
				values.push_back(entity.Value);
		}

		template<typename TContainer>
		std::vector<uint32_t> IterateValues(TContainer& container) {
			std::vector<uint32_t> values;
			IterateValues(container, values);
			return values;
		}
	}

	TRAITS_BASED_TEST(CanProcessFewerElementsThanInBuffer) {
		// Arrange: buffer contains 3 entities, but container only wraps two of them
		std::vector<uint8_t> buffer(100);
		reinterpret_cast<EntityHeader&>(*&buffer[0]) = { 20, 17 };
		reinterpret_cast<EntityHeader&>(*&buffer[24]) = { 30, 25 };
		reinterpret_cast<EntityHeader&>(*&buffer[56]) = { 10, 14 };
		auto pEntities = reinterpret_cast<EntityHeader*>(&buffer[0]);
		auto container = TContainerTraits::MakeContainer(pEntities, 0, 24 + 30);

		// Act:
		auto values = IterateValues(container);

		// Assert:
		EXPECT_EQ(std::vector<uint32_t>({ 17, 25 }), values);
		EXPECT_FALSE(container.hasError());
	}

	namespace {
		struct FirstElementTraits {
			static std::vector<uint8_t> PrepareBuffer(uint32_t size) {
				std::vector<uint8_t> buffer(100);
				reinterpret_cast<EntityHeader&>(*&buffer[0]) = { size, 17 };
				reinterpret_cast<EntityHeader&>(*&buffer[24]) = { 30, 25 };
				reinterpret_cast<EntityHeader&>(*&buffer[56]) = { 10, 14 };
				return buffer;
			}

			static std::vector<uint32_t> ExpectedValues() {
				return {};
			}
		};

		struct MiddleElementTraits {
			static std::vector<uint8_t> PrepareBuffer(uint32_t size) {
				std::vector<uint8_t> buffer(100);
				reinterpret_cast<EntityHeader&>(*&buffer[0]) = { 20, 17 };
				reinterpret_cast<EntityHeader&>(*&buffer[24]) = { size, 25 };
				reinterpret_cast<EntityHeader&>(*&buffer[56]) = { 10, 14 };
				return buffer;
			}

			static std::vector<uint32_t> ExpectedValues() {
				return { 17 };
			}
		};

		struct LastElementTraits {
			static std::vector<uint8_t> PrepareBuffer(uint32_t size) {
				std::vector<uint8_t> buffer(100);
				reinterpret_cast<EntityHeader&>(*&buffer[0]) = { 20, 17 };
				reinterpret_cast<EntityHeader&>(*&buffer[24]) = { 30, 25 };
				reinterpret_cast<EntityHeader&>(*&buffer[56]) = { size, 14 };
				return buffer;
			}

			static std::vector<uint32_t> ExpectedValues() {
				return { 17, 25 };
			}
		};

// there are three sets of tests: first, middle, last
#define POSITIONAL_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits, typename TContainerTraits, typename TPositionalTraits> \
	void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TRAITS_BASED_TEST(TEST_NAME##_First) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TTraits, TContainerTraits, FirstElementTraits>(); } \
	TRAITS_BASED_TEST(TEST_NAME##_Middle) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TTraits, TContainerTraits, MiddleElementTraits>(); } \
	TRAITS_BASED_TEST(TEST_NAME##_Last) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TTraits, TContainerTraits, LastElementTraits>(); } \
	template<typename TTraits, typename TContainerTraits, typename TPositionalTraits> \
	void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		template<typename TContainerTraits, typename TPositionalTraits>
		void AssertShortCircuitOnInsufficientEntitySize(uint32_t size) {
			// Arrange: buffer contains three elements but one is invalid
			auto buffer = TPositionalTraits::PrepareBuffer(size);
			auto pEntities = reinterpret_cast<EntityHeader*>(&buffer[0]);
			auto container = TContainerTraits::MakeContainer(pEntities, 0, buffer.size());

			// Act: an iteration error should be raised
			std::vector<uint32_t> values;
			EXPECT_ITERATION_ERROR(IterateValues(container, values));

			// Assert: only the expected values were extracted before the error
			EXPECT_EQ(TPositionalTraits::ExpectedValues(), values);
			EXPECT_TRUE(container.hasError());
		}
	}

	POSITIONAL_TRAITS_BASED_TEST(AbortsWhenEntitySizeIsZero) {
		AssertShortCircuitOnInsufficientEntitySize<TContainerTraits, TPositionalTraits>(0);
	}

	POSITIONAL_TRAITS_BASED_TEST(AbortsWhenEntityHeaderExtendsBeyondBuffer) {
		AssertShortCircuitOnInsufficientEntitySize<TContainerTraits, TPositionalTraits>(sizeof(EntityHeader) - 1);
	}

	POSITIONAL_TRAITS_BASED_TEST(AbortsWhenEntityExtendsBeyondBuffer) {
		// Assert: buffer contains three elements but one extends beyond buffer
		AssertShortCircuitOnInsufficientEntitySize<TContainerTraits, TPositionalTraits>(101);
	}

	namespace {
		struct PostfixIteratorTraits {
			template<typename TIterator>
			static void Advance(TIterator& iter) {
				iter++;
			}
		};

		struct PrefixIteratorTraits {
			template<typename TIterator>
			static void Advance(TIterator& iter) {
				++iter;
			}
		};

		template<typename TTraits, typename TContainerTraits, typename TIteratorTraits>
		void AssertCannotAdvanceIteratorAfterError() {
			// Arrange: trigger an error by setting the size of the second element to zero
			auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
			entities[1].Size = 0;
			auto container = TContainerTraits::MakeContainer(&entities[0], 3);

			// Act + Assert:
			// - initially no error
			auto iter = TTraits::begin(container);
			EXPECT_EQ(17u, iter->Value);
			EXPECT_FALSE(container.hasError());

			// - error after advancing
			EXPECT_ITERATION_ERROR(TIteratorTraits::Advance(iter));
			EXPECT_TRUE(container.hasError());
			EXPECT_EQ(TTraits::end(container), iter);

			// - cannot advance any further
			EXPECT_THROW(TIteratorTraits::Advance(iter), catapult_out_of_range);
			EXPECT_THROW(TIteratorTraits::Advance(iter), catapult_out_of_range);
		}

		template<typename TTraits, typename TContainerTraits, typename TIteratorTraits>
		void AssertCannotAdvanceIteratorAfterErrorAtEnd() {
			// Arrange: trigger an error by indicating the container size is one byte too large
			auto entities = CreateFixedSizedEntities({ 17, 14 });
			auto container = TContainerTraits::MakeContainer(&entities[0], 0, 2 * sizeof(EntityHeader) + 1);

			// Act + Assert:
			// - initially no error
			auto iter = TTraits::begin(container);
			EXPECT_EQ(17u, iter->Value);
			EXPECT_FALSE(container.hasError());

			// - can advance without error
			TIteratorTraits::Advance(iter);
			EXPECT_EQ(14u, iter->Value);
			EXPECT_FALSE(container.hasError());

			// - error before reaching end
			EXPECT_ITERATION_ERROR(TIteratorTraits::Advance(iter));
			EXPECT_TRUE(container.hasError());
			EXPECT_EQ(TTraits::end(container), iter);

			// - cannot advance any further
			EXPECT_THROW(TIteratorTraits::Advance(iter), catapult_out_of_range);
			EXPECT_THROW(TIteratorTraits::Advance(iter), catapult_out_of_range);
		}
	}

	TRAITS_BASED_TEST(CannotAdvancePostfixIteratorAfterError) {
		AssertCannotAdvanceIteratorAfterError<TTraits, TContainerTraits, PostfixIteratorTraits>();
	}

	TRAITS_BASED_TEST(CannotAdvancePrefixIteratorAfterError) {
		AssertCannotAdvanceIteratorAfterError<TTraits, TContainerTraits, PrefixIteratorTraits>();
	}

	TRAITS_BASED_TEST(CannotAdvancePostfixIteratorAfterErrorAtEnd) {
		AssertCannotAdvanceIteratorAfterErrorAtEnd<TTraits, TContainerTraits, PostfixIteratorTraits>();
	}

	TRAITS_BASED_TEST(CannotAdvancePrefixIteratorAfterErrorAtEnd) {
		AssertCannotAdvanceIteratorAfterErrorAtEnd<TTraits, TContainerTraits, PrefixIteratorTraits>();
	}

	TRAITS_BASED_TEST(BeginAbortsWhenFirstElementHasInvalidSize) {
		// Arrange: trigger an error by setting the size of the first element to zero
		auto entities = CreateFixedSizedEntities({ 17, 25, 14 });
		entities[0].Size = 0;
		auto container = TContainerTraits::MakeContainer(&entities[0], 3);

		// Act + Assert:
		// - initially an error
		EXPECT_ITERATION_ERROR(TTraits::begin(container));
		EXPECT_TRUE(container.hasError());
	}
}}
