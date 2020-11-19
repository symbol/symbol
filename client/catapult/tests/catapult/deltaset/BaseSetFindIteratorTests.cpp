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

#include "catapult/deltaset/BaseSetFindIterator.h"
#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

#define TEST_CLASS BaseSetFindIteratorTests

	namespace {
		template<typename TMutabilityTraits>
		using UnorderedMapTraits = test::BaseTraits<
			test::BaseSetTraits<
				TMutabilityTraits,
				test::UnorderedMapSetTraits<test::SetElementType<TMutabilityTraits>>>>;

		using NonPointerTraits = UnorderedMapTraits<test::MutableElementValueTraits>;
	}

#define FIND_ITERATOR_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_NonPointer) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonPointerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region utils

	namespace {
		template<typename TIteratorTraits>
		void AssertCanCreateUnset() {
			// Act:
			typename TIteratorTraits::IteratorType iter;

			// Assert:
			EXPECT_FALSE(!!iter.get());
		}

		template<typename TIteratorTraits>
		void AssertCanCreateAroundStorageContainer() {
			// Arrange:
			typename TIteratorTraits::SetTraits::SetType set;
			set.emplace(std::make_pair("a", 3), TIteratorTraits::CreateElement("a", 3));

			// Act:
			typename TIteratorTraits::IteratorType iter(set.begin());

			// Assert:
			ASSERT_TRUE(!!iter.get());
			EXPECT_EQ(3u, iter.get()->Value);

			EXPECT_EQ(TIteratorTraits::Is_Const_Accessor, std::is_const_v<std::remove_reference_t<decltype(*iter.get())>>);
		}

		template<typename TIteratorTraits>
		void AssertCanCreateAroundMemoryContainer() {
			// Arrange:
			typename TIteratorTraits::SetTraits::MemorySetType set;
			set.emplace(std::make_pair("a", 3), TIteratorTraits::CreateElement("a", 3));

			// Act:
			typename TIteratorTraits::IteratorType iter(set.begin());

			// Assert:
			ASSERT_TRUE(!!iter.get());
			EXPECT_EQ(3u, iter.get()->Value);

			EXPECT_EQ(TIteratorTraits::Is_Const_Accessor, std::is_const_v<std::remove_reference_t<decltype(*iter.get())>>);
		}

		template<typename TSetTraits>
		struct SetTraitsWithDifferentMemorySetType : public TSetTraits {
		public:
			using MemorySetType = std::vector<typename TSetTraits::SetType::value_type>;

		public:
			using TSetTraits::ToValue;

			static typename TSetTraits::ValueType& ToValue(typename TSetTraits::ValueType& value) {
				return value;
			}
		};

		// difference from AssertCanCreateAroundMemoryContainer is that emplace_back is called instead of emplace
		template<typename TIteratorTraits>
		void AssertCanCreateAroundMemoryContainerDual() {
			// Arrange:
			typename TIteratorTraits::SetTraits::MemorySetType set;
			set.emplace_back(std::make_pair("a", 3), TIteratorTraits::CreateElement("a", 3));

			// Act:
			typename TIteratorTraits::IteratorType iter(set.begin());

			// Assert:
			ASSERT_TRUE(!!iter.get());
			EXPECT_EQ(3u, iter.get()->Value);

			EXPECT_EQ(TIteratorTraits::Is_Const_Accessor, std::is_const_v<std::remove_reference_t<decltype(*iter.get())>>);
		}
	}

	// endregion

	// region BaseSetFindIterator

	namespace {
		template<typename TTraits, typename TSetTraits = typename TTraits::BaseSetTraits::SetTraits>
		struct BaseSetFindIteratorTraits {
		public:
			using FindTraits = typename TTraits::BaseSetTraits::Type::FindTraits;
			using SetTraits = TSetTraits;

			using IteratorType = BaseSetFindIterator<FindTraits, SetTraits>;

			static constexpr auto CreateElement = TTraits::CreateElement;
			static constexpr auto Is_Const_Accessor = true;
		};

		template<typename TTraits>
		using BaseSetFindIteratorDualTraits = BaseSetFindIteratorTraits<
			TTraits,
			SetTraitsWithDifferentMemorySetType<typename TTraits::BaseSetTraits::SetTraits>>;
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetFindIterator_CanCreateUnset_Single) {
		AssertCanCreateUnset<BaseSetFindIteratorTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetFindIterator_CanCreateAroundStorageContainerIterator_Single) {
		AssertCanCreateAroundStorageContainer<BaseSetFindIteratorTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetFindIterator_CanCreateUnset_Dual) {
		AssertCanCreateUnset<BaseSetFindIteratorDualTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetFindIterator_CanCreateAroundStorageContainerIterator_Dual) {
		AssertCanCreateAroundStorageContainer<BaseSetFindIteratorDualTraits<TTraits>>();
	}

	// endregion

	// region BaseSetDeltaFindIterator

	namespace {
		template<typename TTraits, typename TSetTraits = typename TTraits::BaseSetTraits::SetTraits>
		struct BaseSetDeltaFindIteratorTraits {
		public:
			using FindTraits = typename TTraits::BaseSetTraits::Type::FindTraits;
			using SetTraits = TSetTraits;

			using IteratorType = BaseSetDeltaFindIterator<FindTraits, SetTraits>;

			static constexpr auto CreateElement = TTraits::CreateElement;
			static constexpr auto Is_Const_Accessor = false;
		};

		template<typename TTraits>
		using BaseSetDeltaFindIteratorDualTraits = BaseSetDeltaFindIteratorTraits<
			TTraits,
			SetTraitsWithDifferentMemorySetType<typename TTraits::BaseSetTraits::SetTraits>>;
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindIterator_CanCreateUnset_Single) {
		AssertCanCreateUnset<BaseSetDeltaFindIteratorTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindIterator_CanCreateAroundMemoryContainerIterator_Single) {
		AssertCanCreateAroundMemoryContainer<BaseSetDeltaFindIteratorTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindIterator_CanCreateUnset_Dual) {
		AssertCanCreateUnset<BaseSetDeltaFindIteratorDualTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindIterator_CanCreateAroundMemoryContainerIterator_Dual) {
		AssertCanCreateAroundMemoryContainerDual<BaseSetDeltaFindIteratorDualTraits<TTraits>>();
	}

	// endregion

	// region BaseSetDeltaFindConstIterator

	namespace {
		template<typename TTraits, typename TSetTraits = typename TTraits::BaseSetTraits::SetTraits>
		struct BaseSetDeltaFindConstIteratorTraits {
		public:
			using FindTraits = typename TTraits::BaseSetTraits::Type::FindTraits;
			using SetTraits = TSetTraits;

			using IteratorType = BaseSetDeltaFindConstIterator<FindTraits, SetTraits>;

			static constexpr auto CreateElement = TTraits::CreateElement;
			static constexpr auto Is_Const_Accessor = true;
		};

		template<typename TTraits>
		using BaseSetDeltaFindConstIteratorDualTraits = BaseSetDeltaFindConstIteratorTraits<
			TTraits,
			SetTraitsWithDifferentMemorySetType<typename TTraits::BaseSetTraits::SetTraits>>;
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindConstIterator_CanCreateUnset_Single) {
		AssertCanCreateUnset<BaseSetDeltaFindConstIteratorTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindConstIterator_CanCreateAroundStorageContainerIterator_Single) {
		AssertCanCreateAroundStorageContainer<BaseSetDeltaFindConstIteratorTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindConstIterator_CanCreateAroundMemoryContainerIterator_Single) {
		AssertCanCreateAroundMemoryContainer<BaseSetDeltaFindConstIteratorTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindConstIterator_CanCreateUnset_Dual) {
		AssertCanCreateUnset<BaseSetDeltaFindConstIteratorDualTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindConstIterator_CanCreateAroundStorageContainerIterator_Dual) {
		AssertCanCreateAroundStorageContainer<BaseSetDeltaFindConstIteratorDualTraits<TTraits>>();
	}

	FIND_ITERATOR_TRAITS_BASED_TEST(BaseSetDeltaFindConstIterator_CanCreateAroundMemoryContainerIterator_Dual) {
		AssertCanCreateAroundMemoryContainerDual<BaseSetDeltaFindConstIteratorDualTraits<TTraits>>();
	}

	// endregion
}}

