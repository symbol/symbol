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

#include "catapult/deltaset/ConditionalContainer.h"
#include "catapult/deltaset/OrderedSet.h"
#include "catapult/utils/ContainerHelpers.h"
#include "tests/test/other/DeltaElementsTestUtils.h"
#include "tests/TestHarness.h"
#include <unordered_set>

namespace catapult { namespace deltaset {

#define TEST_CLASS ConditionalContainerTests

	namespace {
		// region MapTraits

		struct MapTraits {
		private:
			using Types = test::DeltaElementsTestUtils::Types;

			template<typename TKeyTraits, typename TStorageMap, typename TMemoryMap>
			struct BasicMapTraits {
			public:
				using DeltaElementsWrapper = test::DeltaElementsTestUtils::Wrapper<TMemoryMap>;
				using ContainerType = ConditionalContainer<TKeyTraits, TStorageMap, TMemoryMap>;

			public:
				static ContainerType CreateContainer(ConditionalContainerMode mode) {
					return ContainerType(mode);
				}

				static auto MakeKey(const std::string& name, unsigned int value) {
					return std::make_pair(name, value);
				}

				static const test::MutableTestElement& GetValue(const typename TStorageMap::value_type& pair) {
					return pair.second;
				}

				static void AddElement(TMemoryMap& map, const std::string& name, unsigned int value) {
					map.emplace(std::make_pair(name, value), test::MutableTestElement(name, value));
				}

				static bool Contains(const ContainerType& map, const std::string& name, unsigned int value) {
					return map.cend() != map.find(MakeKey(name, value));
				}
			};

		public:
			using SameUnderlying = BasicMapTraits<Types::StorageTraits::KeyTraits, Types::StorageMapType, Types::StorageMapType>;
			using DiffUnderlying = BasicMapTraits<Types::StorageTraits::KeyTraits, Types::StorageMapType, Types::MemoryMapType>;
		};

		// endregion

		// region SetTraits

		struct SetTraits {
		public:
			struct Types {
			private:
				using ElementType = test::SetElementType<test::MutableElementValueTraits>;

			public:
				using StorageSetType = std::set<ElementType>;
				using MemorySetType = std::unordered_set<ElementType, test::Hasher<ElementType>>;

				// to emulate storage virtualization, use two separate sets (ordered and unordered)
				using StorageTraits = deltaset::SetStorageTraits<StorageSetType, MemorySetType>;
			};

			using DeltaElementsWrapper = test::DeltaElementsTestUtils::Wrapper<Types::MemorySetType>;
			using ContainerType = ConditionalContainer<Types::StorageTraits::KeyTraits, Types::StorageSetType, Types::MemorySetType>;

		public:
			static ContainerType CreateContainer(ConditionalContainerMode mode) {
				return ContainerType(mode);
			}

			static auto MakeKey(const std::string& name, unsigned int value) {
				return test::MutableTestElement(name, value);
			}

			static const test::MutableTestElement& GetValue(const test::MutableTestElement& element) {
				return element;
			}

			static void AddElement(Types::MemorySetType& set, const std::string& name, unsigned int value) {
				set.emplace(name, value);
			}

			static bool Contains(const ContainerType& set, const std::string& name, unsigned int value) {
				return set.cend() != set.find(MakeKey(name, value));
			}
		};

		// endregion

		constexpr auto StorageMode = ConditionalContainerMode::Storage;
		constexpr auto MemoryMode = ConditionalContainerMode::Memory;
	}
}}

namespace catapult { namespace test {

	using StorageSetType = deltaset::SetTraits::Types::StorageSetType;
	using MemorySetType = deltaset::SetTraits::Types::MemorySetType;

	void PruneBaseSet(StorageSetType& elements, const deltaset::PruningBoundary<StorageSetType::value_type>& pruningBoundary);
	void PruneBaseSet(StorageSetType& elements, const deltaset::PruningBoundary<StorageSetType::value_type>& pruningBoundary) {
		deltaset::PruneBaseSet(elements, pruningBoundary);
	}

	// need custom PruneBaseSet for unordered_set, which does not support lower_bound
	void PruneBaseSet(MemorySetType& elements, const deltaset::PruningBoundary<MemorySetType::value_type>& pruningBoundary);
	void PruneBaseSet(MemorySetType& elements, const deltaset::PruningBoundary<MemorySetType::value_type>& pruningBoundary) {
		utils::map_erase_if(elements, [pruningBoundary](const auto& element) {
			return element < pruningBoundary.value();
		});
	}
}}

namespace catapult { namespace deltaset {

	// region traits based tests

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<ConditionalContainerMode Mode, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_StorageMap) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StorageMode, MapTraits::SameUnderlying>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MemoryMap) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MemoryMode, MapTraits::SameUnderlying>(); } \
	TEST(TEST_CLASS, TEST_NAME##_StorageMapDiff) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StorageMode, MapTraits::DiffUnderlying>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MemoryMapDiff) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MemoryMode, MapTraits::DiffUnderlying>(); } \
	\
	TEST(TEST_CLASS, TEST_NAME##_StorageSet) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StorageMode, SetTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MemorySet) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MemoryMode, SetTraits>(); } \
	template<ConditionalContainerMode Mode, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_TEST(CanCreateContainer) {
		// Act:
		auto container = TTraits::CreateContainer(Mode);

		// Assert:
		EXPECT_TRUE(container.empty());
		EXPECT_EQ(0u, container.size());
	}

	TRAITS_BASED_TEST(CanDefaultConstructContainerIterators) {
		// Act:
		using ContainerType = decltype(TTraits::CreateContainer(Mode));
		typename ContainerType::iterator iter;
		typename ContainerType::const_iterator citer;

		// Assert: no exceptions
	}

	TRAITS_BASED_TEST(CanInsertElements) {
		// Arrange:
		auto container = TTraits::CreateContainer(Mode);

		typename TTraits::DeltaElementsWrapper wrapper;
		TTraits::AddElement(wrapper.Added, "alpha", 5);
		TTraits::AddElement(wrapper.Added, "gamma", 7);

		// Act:
		container.update(wrapper.deltas());

		// Assert:
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());

		EXPECT_TRUE(TTraits::Contains(container, "alpha", 5));
		EXPECT_TRUE(TTraits::Contains(container, "gamma", 7));
	}

	TRAITS_BASED_TEST(CanInsertElementsViaFreeFunction) {
		// Arrange:
		auto container = TTraits::CreateContainer(Mode);

		typename TTraits::DeltaElementsWrapper wrapper;
		TTraits::AddElement(wrapper.Added, "alpha", 5);
		TTraits::AddElement(wrapper.Added, "gamma", 7);

		// Act:
		UpdateSet(container, wrapper.deltas());

		// Assert:
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());

		EXPECT_TRUE(TTraits::Contains(container, "alpha", 5));
		EXPECT_TRUE(TTraits::Contains(container, "gamma", 7));
	}

	TRAITS_BASED_TEST(FindReturnsValidIteratorForMatchingElement) {
		// Arrange:
		auto container = TTraits::CreateContainer(Mode);

		typename TTraits::DeltaElementsWrapper wrapper;
		TTraits::AddElement(wrapper.Added, "alpha", 5);
		TTraits::AddElement(wrapper.Added, "gamma", 7);
		container.update(wrapper.deltas());

		// Act:
		auto iter = container.find(TTraits::MakeKey("gamma", 7));

		// Assert:
		ASSERT_NE(container.cend(), iter);
		EXPECT_EQ("gamma", TTraits::GetValue(*iter).Name);
		EXPECT_EQ(&*iter, iter.operator->());
	}

	TRAITS_BASED_TEST(FindReturnsDifferentIteratorsForDifferentElements) {
		// Arrange:
		auto container = TTraits::CreateContainer(Mode);

		typename TTraits::DeltaElementsWrapper wrapper;
		TTraits::AddElement(wrapper.Added, "alpha", 5);
		TTraits::AddElement(wrapper.Added, "gamma", 7);
		container.update(wrapper.deltas());

		// Act:
		auto iter1 = container.find(TTraits::MakeKey("alpha", 5));
		auto iter2 = container.find(TTraits::MakeKey("gamma", 7));

		// Assert:
		EXPECT_NE(iter1, iter2);

		ASSERT_NE(container.cend(), iter1);
		EXPECT_EQ("alpha", TTraits::GetValue(*iter1).Name);
		EXPECT_EQ(&*iter1, iter1.operator->());

		ASSERT_NE(container.cend(), iter2);
		EXPECT_EQ("gamma", TTraits::GetValue(*iter2).Name);
		EXPECT_EQ(&*iter2, iter2.operator->());
	}

	TRAITS_BASED_TEST(FindReturnsEndWhenThereIsNoMatchingElement) {
		// Arrange:
		auto container = TTraits::CreateContainer(Mode);

		typename TTraits::DeltaElementsWrapper wrapper;
		TTraits::AddElement(wrapper.Added, "alpha", 5);
		TTraits::AddElement(wrapper.Added, "gamma", 7);
		container.update(wrapper.deltas());

		// Act:
		auto iter = container.find(TTraits::MakeKey("zeta", 5));

		// Assert:
		EXPECT_EQ(container.cend(), iter);
	}

	// endregion

	// region set traits based pruning test

#define TRAITS_BASED_SET_TEST(TEST_NAME) \
	template<ConditionalContainerMode Mode, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_StorageSet) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StorageMode, SetTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MemorySet) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MemoryMode, SetTraits>(); } \
	template<ConditionalContainerMode Mode, typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	namespace {
		template<ConditionalContainerMode Mode, typename TTraits>
		auto CreateContainerSeededForPruningTest() {
			auto container = TTraits::CreateContainer(Mode);

			typename TTraits::DeltaElementsWrapper wrapper;
			TTraits::AddElement(wrapper.Added, "alpha", 5);
			TTraits::AddElement(wrapper.Added, "zeta", 100);
			TTraits::AddElement(wrapper.Added, "beta", 6);
			TTraits::AddElement(wrapper.Added, "gamma", 7);
			container.update(wrapper.deltas());

			return container;
		}
	}

	TRAITS_BASED_SET_TEST(CanPruneElements) {
		// Arrange:
		auto container = CreateContainerSeededForPruningTest<Mode, TTraits>();

		// Act:
		container.prune(TTraits::MakeKey("gamma", 7));

		// Assert:
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());

		EXPECT_TRUE(TTraits::Contains(container, "gamma", 7));
		EXPECT_TRUE(TTraits::Contains(container, "zeta", 100));
	}

	TRAITS_BASED_SET_TEST(CanPruneElementsViaFreeFunction) {
		// Arrange:
		auto container = CreateContainerSeededForPruningTest<Mode, TTraits>();

		// Act:
		PruneBaseSet(container, TTraits::MakeKey("gamma", 7));

		// Assert:
		EXPECT_FALSE(container.empty());
		EXPECT_EQ(2u, container.size());

		EXPECT_TRUE(TTraits::Contains(container, "gamma", 7));
		EXPECT_TRUE(TTraits::Contains(container, "zeta", 100));
	}

	// endregion

	// region constructor arguments

	TEST(TEST_CLASS, ConstructorArgumentsAreForwardedToUnderlyingStorageContainer) {
		// Arrange: create an underlying map with two elements
		MapTraits::DiffUnderlying::ContainerType::StorageSetType underlyingContainer{
			std::make_pair(std::make_pair("alpha", 5), test::MutableTestElement("alpha", 5)),
			std::make_pair(std::make_pair("gamma", 7), test::MutableTestElement("gamma", 7))
		};

		// Act: create a container around it
		MapTraits::DiffUnderlying::ContainerType container(ConditionalContainerMode::Storage, underlyingContainer);

		// Assert: storage should be seeded because storage container is forwarded map argument (copy constructor)
		EXPECT_EQ(2u, container.size());
		EXPECT_TRUE(MapTraits::DiffUnderlying::Contains(container, "alpha", 5));
		EXPECT_TRUE(MapTraits::DiffUnderlying::Contains(container, "gamma", 7));
	}

	TEST(TEST_CLASS, ConstructorArgumentsAreNotForwardedToUnderlyingMemoryContainer) {
		// Arrange: create an underlying map with two elements
		MapTraits::DiffUnderlying::ContainerType::StorageSetType underlyingContainer{
			std::make_pair(std::make_pair("alpha", 5), test::MutableTestElement("alpha", 5)),
			std::make_pair(std::make_pair("gamma", 7), test::MutableTestElement("gamma", 7))
		};

		// Act: create a container around it
		MapTraits::DiffUnderlying::ContainerType container(ConditionalContainerMode::Memory, underlyingContainer);

		// Assert: storage should be empty because memory container is not forwarded map argument
		EXPECT_EQ(0u, container.size());
		EXPECT_FALSE(MapTraits::DiffUnderlying::Contains(container, "alpha", 5));
		EXPECT_FALSE(MapTraits::DiffUnderlying::Contains(container, "gamma", 7));
	}

	// endregion

	// region iterable

	TEST(TEST_CLASS, StorageBasedCacheIsNotIterable) {
		// Act:
		MapTraits::DiffUnderlying::ContainerType container(ConditionalContainerMode::Storage);

		// Assert:
		EXPECT_FALSE(IsSetIterable(container));
		EXPECT_THROW(SelectIterableSet(container), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, MemoryBasedCacheIsIterable) {
		// Act:
		MapTraits::DiffUnderlying::ContainerType container(ConditionalContainerMode::Memory);

		// Assert:
		EXPECT_TRUE(IsSetIterable(container));
		EXPECT_NO_THROW(SelectIterableSet(container));
	}

	// endregion
}}
