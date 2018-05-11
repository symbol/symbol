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

#include "catapult/cache/CacheMixins.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/utils/Hashers.h"
#include "tests/TestHarness.h"
#include <unordered_map>

namespace catapult { namespace cache {

#define TEST_CLASS CacheMixinsTests

	namespace {
		// fake cache that indexes strings by size
		struct TestCacheDescriptor {
			static constexpr auto Name = "TestCache";

			using KeyType = int;
			using ValueType = std::string;

			static KeyType GetKeyFromValue(const ValueType& value) {
				return static_cast<int>(value.size());
			}
		};

		template<typename TBaseSet>
		class BaseSetTypeWrapper : public TBaseSet {
		public:
			BaseSetTypeWrapper() : TBaseSet(deltaset::ConditionalContainerMode::Memory, m_database, 0)
			{}

		private:
			CacheDatabase m_database;
		};

		using BasicTypes = MutableUnorderedMapAdapter<TestCacheDescriptor>;
		using BaseSetType = BaseSetTypeWrapper<BasicTypes::BaseSetType>;

		auto SeedThreeDelta(BaseSetType& set) {
			auto pDelta = set.rebase();
			pDelta->insert("a");
			pDelta->insert("ccc");
			pDelta->insert("fffff");
			return pDelta;
		}

		void SeedThree(BaseSetType& set) {
			auto pDelta = SeedThreeDelta(set);
			set.commit();
		}
	}

	// region SizeMixin

	TEST(TEST_CLASS, SizeMixin_ReturnsCacheSizeWhenEmpty) {
		// Arrange:
		BaseSetType set;
		auto mixin = SizeMixin<BaseSetType>(set);

		// Act:
		auto size = mixin.size();

		// Assert:
		EXPECT_EQ(0u, size);
	}

	TEST(TEST_CLASS, SizeMixin_ReturnsCacheSizeWhenNotEmpty) {
		// Arrange:
		BaseSetType set;
		SeedThree(set);
		auto mixin = SizeMixin<BaseSetType>(set);

		// Act:
		auto size = mixin.size();

		// Assert:
		EXPECT_EQ(3u, size);
	}

	// endregion

	// region ContainsMixin

	TEST(TEST_CLASS, ContainsMixin_ReturnsFalseForKeysNotInCache) {
		// Arrange:
		BaseSetType set;
		SeedThree(set);
		auto mixin = ContainsMixin<BaseSetType, TestCacheDescriptor>(set);

		// Act:
		auto contains1 = mixin.contains(4);
		auto contains2 = mixin.contains(2);

		// Assert:
		EXPECT_FALSE(contains1);
		EXPECT_FALSE(contains2);
	}

	TEST(TEST_CLASS, ContainsMixin_ReturnsTrueForKeysInCache) {
		// Arrange:
		BaseSetType set;
		SeedThree(set);
		auto mixin = ContainsMixin<BaseSetType, TestCacheDescriptor>(set);

		// Act:
		auto contains1 = mixin.contains(3);
		auto contains2 = mixin.contains(5);

		// Assert:
		EXPECT_TRUE(contains1);
		EXPECT_TRUE(contains2);
	}

	// endregion

	// region IterationMixin

	namespace {
		struct TestSetCacheDescriptor {
			using KeyType = std::string;
			using ValueType = std::string;
		};

		using OrderedSetBasicTypes = MutableOrderedSetAdapter<TestSetCacheDescriptor>;
		using OrderedBaseSetType = BaseSetTypeWrapper<OrderedSetBasicTypes::BaseSetType>;

		void Commit(BaseSetType& set) {
			set.commit();
		}

		void Commit(OrderedSetBasicTypes::BaseSetType& set) {
			set.commit(deltaset::PruningBoundary<std::string>());
		}

		template<typename TBaseSet, typename TAction>
		void RunIterationTest(const std::vector<std::string>& values, TAction action) {
			// Arrange: seed the set
			TBaseSet set;
			auto pDelta = set.rebase();
			for (const auto& value : values)
				pDelta->insert(value);

			Commit(set);

			// Sanity:
			CATAPULT_LOG(debug) << "set is seeded with " << set.size() << " elements";
			EXPECT_EQ(values.size(), set.size());

			// Act + Assert:
			action(set);
		}

		class MapIterationTests {
		public:
			static void AssertIterationWithIterators(const std::vector<std::string>& values) {
				AssertIteration(values, [](const auto& mixin) {
					// Act:
					std::unordered_map<int, std::string> contents;
					auto pIterableView = mixin.tryMakeIterableView();
					for (const auto& pair : *pIterableView)
						contents.insert(pair);

					return contents;
				});
			}

		private:
			template<typename TCacheContentsAccessor>
			static void AssertIteration(const std::vector<std::string>& values, TCacheContentsAccessor cacheContentsAccessor) {
				// Arrange:
				RunIterationTest<BaseSetType>(values, [&values, cacheContentsAccessor](const auto& set) {
					auto mixin = IterationMixin<BaseSetType>(set);

					// Act: iterate over the cache and extract its contents
					auto contents = cacheContentsAccessor(mixin);

					// Assert:
					ASSERT_EQ(values.size(), contents.size());
					for (const auto& value : values) {
						auto key = static_cast<int>(value.size());
						auto contentsIter = contents.find(key);

						auto message = "value " + value;
						ASSERT_TRUE(contents.cend() != contentsIter) << message;
						EXPECT_EQ(key, contentsIter->first) << message;
						EXPECT_EQ(value, contentsIter->second) << message;

						contents.erase(contentsIter);
					}

					// Sanity:
					EXPECT_TRUE(contents.empty());
				});
			}
		};

		class SetIterationTests {
		public:
			static void AssertIterationWithIterators(const std::vector<std::string>& values) {
				AssertIteration(values, [](const auto& mixin) {
					// Act:
					std::set<std::string> contents;
					auto pIterableView = mixin.tryMakeIterableView();
					for (const auto& value : *pIterableView)
						contents.insert(value);

					return contents;
				});
			}

		private:
			template<typename TCacheContentsAccessor>
			static void AssertIteration(const std::vector<std::string>& values, TCacheContentsAccessor cacheContentsAccessor) {
				// Arrange:
				RunIterationTest<OrderedBaseSetType>(values, [&values, cacheContentsAccessor](const auto& set) {
					auto mixin = IterationMixin<OrderedBaseSetType>(set);

					// Act: iterate over the cache and extract its contents
					auto contents = cacheContentsAccessor(mixin);

					// Assert:
					ASSERT_EQ(values.size(), contents.size());
					for (const auto& value : values) {
						auto contentsIter = contents.find(value);

						auto message = "value " + value;
						ASSERT_TRUE(contents.cend() != contentsIter) << message;
						EXPECT_EQ(value, *contentsIter) << message;

						contents.erase(contentsIter);
					}

					// Sanity:
					EXPECT_TRUE(contents.empty());
				});
			}
		};

		struct MapIterationIterators {
			static constexpr auto AssertIteration = MapIterationTests::AssertIterationWithIterators;
		};

		struct SetIterationIterators {
			static constexpr auto AssertIteration = SetIterationTests::AssertIterationWithIterators;
		};
	}

#define ITERATION_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, IterationMixin_##TEST_NAME##_Map_Iterators) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MapIterationIterators>(); } \
	TEST(TEST_CLASS, IterationMixin_##TEST_NAME##_Set_Iterators) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<SetIterationIterators>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ITERATION_TRAITS_BASED_TEST(CanIterate_ZeroElements) {
		// Assert:
		TTraits::AssertIteration({});
	}

	ITERATION_TRAITS_BASED_TEST(CanIterate_SingleElement) {
		// Assert:
		TTraits::AssertIteration({ "a" });
	}

	ITERATION_TRAITS_BASED_TEST(CanIterate_MultipleElements) {
		// Assert:
		TTraits::AssertIteration({ "a", "ccc", "fffff" });
	}

	namespace {
		struct NonSetType {};

		bool IsBaseSetIterable(const NonSetType&) {
			return false;
		}
	}

	TEST(TEST_CLASS, IterationMixin_CannotIterateWhenIterationIsNotSupported) {
		// Arrange: use a fake type that doesn't support iteration
		NonSetType set;
		auto mixin = IterationMixin<NonSetType>(set);

		// Act:
		auto pIterableView = mixin.tryMakeIterableView();

		// Assert:
		EXPECT_FALSE(!!pIterableView);
	}

	// endregion

	// region ConstAccessorMixin / MutableAccessorMixin

	namespace {
		// mapping is from string to first chararcter
		template<typename TDest>
		struct FirstCharacterAdapter {
			using AdaptedValueType = TDest;

			template<typename TSource>
			static TDest& Adapt(TSource& value) {
				return value[0];
			}
		};

		struct ConstNoAdaptAccessor {
			template<typename TAction>
			static void RunAccessorTest(TAction action) {
				// Arrange:
				BaseSetType set;
				SeedThree(set);
				auto mixin = ConstAccessorMixin<BaseSetType, TestCacheDescriptor>(set);

				// Act + Assert:
				action(mixin);
			}

			static void AssertSuccessValues(const std::string& value1, const std::string& value2) {
				EXPECT_EQ("ccc", value1);
				EXPECT_EQ("fffff", value2);
			}
		};

		struct ConstAdaptAccessor {
			template<typename TAction>
			static void RunAccessorTest(TAction action) {
				// Arrange:
				BaseSetType set;
				SeedThree(set);
				auto mixin = ConstAccessorMixin<BaseSetType, TestCacheDescriptor, FirstCharacterAdapter<const char>>(set);

				// Act + Assert:
				action(mixin);
			}

			static void AssertSuccessValues(char value1, char value2) {
				EXPECT_EQ('c', value1);
				EXPECT_EQ('f', value2);
			}
		};

		struct MutableNoAdaptAccessor {
			template<typename TAction>
			static void RunAccessorTest(TAction action) {
				// Arrange:
				BaseSetType set;
				auto pDelta = SeedThreeDelta(set);
				auto mixin = MutableAccessorMixin<BaseSetType::DeltaType, TestCacheDescriptor>(*pDelta);

				// Act + Assert:
				action(mixin);
			}

			static void AssertSuccessValues(const std::string& value1, const std::string& value2) {
				EXPECT_EQ("ccc", value1);
				EXPECT_EQ("fffff", value2);
			}
		};

		struct MutableAdaptAccessor {
			template<typename TAction>
			static void RunAccessorTest(TAction action) {
				// Arrange:
				BaseSetType set;
				auto pDelta = SeedThreeDelta(set);
				auto mixin = MutableAccessorMixin<BaseSetType::DeltaType, TestCacheDescriptor, FirstCharacterAdapter<char>>(*pDelta);

				// Act + Assert:
				action(mixin);
			}

			static void AssertSuccessValues(char value1, char value2) {
				EXPECT_EQ('c', value1);
				EXPECT_EQ('f', value2);
			}
		};
	}

#define ACCESSOR_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, ConstAccessorMixin_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstNoAdaptAccessor>(); } \
	TEST(TEST_CLASS, ConstAccessorMixin_##TEST_NAME##_Adapt) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstAdaptAccessor>(); } \
	TEST(TEST_CLASS, MutableAccessorMixin_##TEST_NAME) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MutableNoAdaptAccessor>(); } \
	TEST(TEST_CLASS, MutableAccessorMixin_##TEST_NAME##_Adapt) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MutableAdaptAccessor>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ACCESSOR_TRAITS_BASED_TEST(GetThrowsForKeysNotInCache) {
		// Arrange:
		TTraits::RunAccessorTest([](auto& mixin) {
			// Act + Assert:
			EXPECT_THROW(mixin.get(4), catapult_invalid_argument);
			EXPECT_THROW(mixin.get(2), catapult_invalid_argument);
		});
	}

	ACCESSOR_TRAITS_BASED_TEST(GetReturnsValuesForKeysInCache) {
		// Arrange:
		TTraits::RunAccessorTest([](auto& mixin) {
			// Act:
			const auto& value1 = mixin.get(3);
			const auto& value2 = mixin.get(5);

			// Assert:
			TTraits::AssertSuccessValues(value1, value2);
		});
	}

	ACCESSOR_TRAITS_BASED_TEST(TryGetReturnsNullptrForKeysNotInCache) {
		// Arrange:
		TTraits::RunAccessorTest([](auto& mixin) {
			// Act:
			const auto* pValue1 = mixin.tryGet(4);
			const auto* pValue2 = mixin.tryGet(2);

			// Assert:
			EXPECT_FALSE(!!pValue1);
			EXPECT_FALSE(!!pValue2);
		});
	}

	ACCESSOR_TRAITS_BASED_TEST(TryGetReturnsValuesForKeysInCache) {
		// Arrange:
		TTraits::RunAccessorTest([](auto& mixin) {
			// Act:
			const auto* pValue1 = mixin.tryGet(3);
			const auto* pValue2 = mixin.tryGet(5);

			// Assert:
			ASSERT_TRUE(!!pValue1);
			ASSERT_TRUE(!!pValue2);

			TTraits::AssertSuccessValues(*pValue1, *pValue2);
		});
	}

	// endregion

	// region ActivePredicateMixin

	namespace {
		struct ConditionallyActiveValue {
			int Key;
			Height ActiveHeight;

			bool isActive(Height height) const {
				return ActiveHeight == height;
			}
		};

		struct TestConditionallyActiveCacheDescriptor {
			using KeyType = int;
			using ValueType = ConditionallyActiveValue;

			static KeyType GetKeyFromValue(const ValueType& value) {
				return value.Key;
			}
		};

		using ConditionallyActiveTypes = MutableUnorderedMapAdapter<TestConditionallyActiveCacheDescriptor>;
		using ConditionallyActiveBaseSet = BaseSetTypeWrapper<ConditionallyActiveTypes::BaseSetType>;

		template<typename TAction>
		void RunActivePredicateMixinTest(TAction action) {
			// Arrange:
			ConditionallyActiveBaseSet set;
			{
				auto pDelta = set.rebase();
				pDelta->insert({ 123, Height(456) });
				set.commit();
			}

			auto mixin = ActivePredicateMixin<ConditionallyActiveBaseSet, TestConditionallyActiveCacheDescriptor>(set);

			// Act + Assert:
			action(mixin);
		}
	}

	TEST(TEST_CLASS, ActivePredicateMixin_IsActiveReturnsTrueWhenValueIsPresentAndActive) {
		// Act:
		RunActivePredicateMixinTest([](const auto& mixin) {
			// Assert:
			EXPECT_TRUE(mixin.isActive(123, Height(456)));
		});
	}

	TEST(TEST_CLASS, ActivePredicateMixin_IsActiveReturnsFalseWhenValueIsPresentAndInactive) {
		// Act:
		RunActivePredicateMixinTest([](const auto& mixin) {
			// Assert:
			EXPECT_FALSE(mixin.isActive(123, Height(455)));
			EXPECT_FALSE(mixin.isActive(123, Height(457)));
		});
	}

	TEST(TEST_CLASS, ActivePredicateMixin_IsActiveReturnsFalseWhenValueIsNotPresent) {
		// Act:
		RunActivePredicateMixinTest([](const auto& mixin) {
			// Assert:
			EXPECT_FALSE(mixin.isActive(122, Height(456)));
			EXPECT_FALSE(mixin.isActive(124, Height(456)));
		});
	}

	// endregion

	// region BasicInsertRemoveMixin

	TEST(TEST_CLASS, BasicInsertRemoveMixin_CanInsertValueIntoCache) {
		// Arrange:
		BaseSetType set;
		auto pDelta = SeedThreeDelta(set);
		auto mixin = BasicInsertRemoveMixin<BaseSetType::DeltaType, TestCacheDescriptor>(*pDelta);

		// Sanity:
		EXPECT_EQ(3u, pDelta->size());
		EXPECT_FALSE(pDelta->contains(4));

		// Act:
		mixin.insert("eeee");

		// Assert:
		EXPECT_EQ(4u, pDelta->size());
		EXPECT_TRUE(pDelta->contains(4));
	}

	TEST(TEST_CLASS, BasicInsertRemoveMixin_CannotInsertExistingValueIntoCache) {
		// Arrange:
		BaseSetType set;
		auto pDelta = SeedThreeDelta(set);
		auto mixin = BasicInsertRemoveMixin<BaseSetType::DeltaType, TestCacheDescriptor>(*pDelta);

		// Sanity:
		EXPECT_EQ(3u, pDelta->size());
		EXPECT_TRUE(pDelta->contains(3));

		// Act + Assert:
		EXPECT_THROW(mixin.insert("ccc"), catapult_invalid_argument);

		// Sanity:
		EXPECT_EQ(3u, pDelta->size());
		EXPECT_TRUE(pDelta->contains(3));
	}

	TEST(TEST_CLASS, BasicInsertRemoveMixin_CanRemoveValueFromCache) {
		// Arrange:
		BaseSetType set;
		auto pDelta = SeedThreeDelta(set);
		auto mixin = BasicInsertRemoveMixin<BaseSetType::DeltaType, TestCacheDescriptor>(*pDelta);

		// Sanity:
		EXPECT_EQ(3u, pDelta->size());
		EXPECT_TRUE(pDelta->contains(3));

		// Act:
		mixin.remove(3);

		// Assert:
		EXPECT_EQ(2u, pDelta->size());
		EXPECT_FALSE(pDelta->contains(3));
	}

	TEST(TEST_CLASS, BasicInsertRemoveMixin_CannotRemoveNonExistentValueFromCache) {
		// Arrange:
		BaseSetType set;
		auto pDelta = SeedThreeDelta(set);
		auto mixin = BasicInsertRemoveMixin<BaseSetType::DeltaType, TestCacheDescriptor>(*pDelta);

		// Sanity:
		EXPECT_EQ(3u, pDelta->size());
		EXPECT_FALSE(pDelta->contains(4));

		// Act + Assert:
		EXPECT_THROW(mixin.remove(4), catapult_invalid_argument);

		// Sanity
		EXPECT_EQ(3u, pDelta->size());
		EXPECT_FALSE(pDelta->contains(4));
	}

	TEST(TEST_CLASS, BasicInsertRemoveMixin_CannotRemoveValueFromCacheMultipleTimes) {
		// Arrange:
		BaseSetType set;
		auto pDelta = SeedThreeDelta(set);
		auto mixin = BasicInsertRemoveMixin<BaseSetType::DeltaType, TestCacheDescriptor>(*pDelta);

		// - remove once
		mixin.remove(3);

		// Sanity:
		EXPECT_EQ(2u, pDelta->size());
		EXPECT_FALSE(pDelta->contains(3));

		// Act + Assert:
		EXPECT_THROW(mixin.remove(3), catapult_invalid_argument);

		// Sanity:
		EXPECT_EQ(2u, pDelta->size());
		EXPECT_FALSE(pDelta->contains(3));
	}

	TEST(TEST_CLASS, BasicInsertRemoveMixin_CanReinsertRemovedValuesIntoCache) {
		// Arrange:
		BaseSetType set;
		auto pDelta = SeedThreeDelta(set);
		auto mixin = BasicInsertRemoveMixin<BaseSetType::DeltaType, TestCacheDescriptor>(*pDelta);

		// Sanity:
		EXPECT_EQ(3u, pDelta->size());
		EXPECT_TRUE(pDelta->contains(3));

		// Act:
		mixin.remove(3);
		mixin.insert("ddd");

		// Assert:
		EXPECT_EQ(3u, pDelta->size());
		ASSERT_TRUE(pDelta->contains(3));
		EXPECT_EQ("ddd", *pDelta->find(3));
	}

	// endregion

	// region HeightBasedPruningMixin

	namespace {
		// int grouped by Height
		using TestIdentifierGroup = utils::IdentifierGroup<int, Height, std::hash<int>>;

		struct TestHeightGroupedCacheDescriptor {
			using KeyType = Height;
			using ValueType = TestIdentifierGroup;

			static KeyType GetKeyFromValue(const ValueType& value) {
				return value.key();
			}
		};

		using HeightGroupedTypes = MutableUnorderedMapAdapter<TestHeightGroupedCacheDescriptor, utils::BaseValueHasher<Height>>;
		using HeightGroupedBaseSetType = BaseSetTypeWrapper<HeightGroupedTypes::BaseSetType>;

		template<typename TAction>
		void RunHeightBasedPruningMixinTest(TAction action) {
			// Arrange:
			BaseSetType set;
			auto pDelta = SeedThreeDelta(set);

			HeightGroupedBaseSetType heightGroupedSet;
			auto pHeightGroupedDelta = heightGroupedSet.rebase();

			// - add group with 2 known (in set) ids at height 3
			pHeightGroupedDelta->insert(TestIdentifierGroup(Height(3)));
			auto pGroup1 = pHeightGroupedDelta->find(Height(3));
			pGroup1->add(1);
			pGroup1->add(5);

			// - add group with 1 known and 2 unknown (not in set) ids at height 7
			pHeightGroupedDelta->insert(TestIdentifierGroup(Height(7)));
			auto pGroup2 = pHeightGroupedDelta->find(Height(7));
			pGroup2->add(2);
			pGroup2->add(3);
			pGroup2->add(4);

			using HeightBasedPruningMixin = HeightBasedPruningMixin<BaseSetType::DeltaType, HeightGroupedBaseSetType::DeltaType>;
			auto mixin = HeightBasedPruningMixin(*pDelta, *pHeightGroupedDelta);

			// Sanity:
			EXPECT_EQ(3u, pDelta->size());
			EXPECT_EQ(2u, pHeightGroupedDelta->size());

			// Act + Assert:
			action(mixin, *pDelta, *pHeightGroupedDelta);
		}
	}

	TEST(TEST_CLASS, HeightBasedPruningMixin_NothingIsRemovedWhenNoIdentifiersAtHeight) {
		// Arrange:
		RunHeightBasedPruningMixinTest([](auto& mixin, const auto& set, const auto& heightGroupedSet) {
			// Sanity:
			EXPECT_FALSE(heightGroupedSet.contains(Height(5)));

			// Act: prune at a height without any identifiers
			mixin.prune(Height(5));

			// Assert: set was not modified
			EXPECT_EQ(3u, set.size());

			// - height grouped set was not modified
			EXPECT_EQ(2u, heightGroupedSet.size());
		});
	}

	TEST(TEST_CLASS, HeightBasedPruningMixin_AllIdentifiersAtHeightAreRemoved) {
		// Arrange:
		RunHeightBasedPruningMixinTest([](auto& mixin, const auto& set, const auto& heightGroupedSet) {
			// Act: prune at a height with known identifiers
			mixin.prune(Height(3));

			// Assert: two elements were removed
			EXPECT_EQ(1u, set.size());
			EXPECT_TRUE(set.contains(3));

			// - one grouping was removed
			EXPECT_EQ(1u, heightGroupedSet.size());
			EXPECT_TRUE(heightGroupedSet.contains(Height(7)));
		});
	}

	TEST(TEST_CLASS, HeightBasedPruningMixin_UnknownIdentifiersAtHeightAreIgnored) {
		// Arrange:
		RunHeightBasedPruningMixinTest([](auto& mixin, const auto& set, const auto& heightGroupedSet) {
			// Act: prune at a height with a mix of known and unknown identifiers
			mixin.prune(Height(7));

			// Assert: one element was removed
			EXPECT_EQ(2u, set.size());
			EXPECT_TRUE(set.contains(1));
			EXPECT_TRUE(set.contains(5));

			// - one grouping was removed
			EXPECT_EQ(1u, heightGroupedSet.size());
			EXPECT_TRUE(heightGroupedSet.contains(Height(3)));
		});
	}

	// endregion
}}
