#pragma once
#include "BaseSetTestsInclude.h"
#include "catapult/exceptions.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

#define DEFINE_SIMPLE_TEST(TEST_NAME, TYPES) \
	template<typename TTraits> void TRAITS_TEST_NAME(BaseSetTests, TEST_NAME)(); \
	TYPES(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(BaseSetTests, TEST_NAME)()

#define REGISTER_TEST(TEST_NAME, TRAITS_TYPE, TRAITS_NAME) \
	TEST(BaseSetTests, TEST_NAME##_##TRAITS_NAME) { TRAITS_TEST_NAME(BaseSetTests, TEST_NAME)<TRAITS_TYPE>(); }

#define REGISTER_DELTA_TESTS(TEST_NAME) \
	REGISTER_DELTA_MUTABLE_TYPES(TEST_NAME); \
	REGISTER_DELTA_IMMUTABLE_TYPES(TEST_NAME); \

#define REGISTER_NON_DELTA_TESTS(TEST_NAME) \
	REGISTER_NON_DELTA_MUTABLE_TYPES(TEST_NAME); \
	REGISTER_NON_DELTA_IMMUTABLE_TYPES(TEST_NAME); \

#define REGISTER_ALL_TESTS(TEST_NAME) \
	REGISTER_DELTA_TESTS(TEST_NAME); \
	REGISTER_NON_DELTA_TESTS(TEST_NAME);

#define DELTA_MUTABLE_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_DELTA_MUTABLE_TYPES)
#define DELTA_IMMUTABLE_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_DELTA_IMMUTABLE_TYPES)
#define NON_DELTA_MUTABLE_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_NON_DELTA_MUTABLE_TYPES)

#define DELTA_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_DELTA_TESTS)
#define NON_DELTA_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_NON_DELTA_TESTS)
#define ALL_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_ALL_TESTS)

	// region multi-test helpers

	namespace {
		/// Dereferences pointer \a pEntity.
		template<typename T>
		T Unwrap(T* pEntity) {
			return *pEntity;
		}

		template<typename T>
		T Unwrap(const std::shared_ptr<T>& pEntity) {
			return *pEntity;
		}

		/// Returns \c true if T is a shared_ptr, \c false otherwise.
		template<typename T>
		bool IsWrapped(T*) {
			return false;
		}

		template<typename T>
		bool IsWrapped(const std::shared_ptr<T>&) {
			return true;
		}

		/// Returns \c true if container is a map.
		template<typename T>
		bool IsMap(const T&) {
			return false;
		}

		template<typename TKey, typename TValue, typename THash>
		bool IsMap(const std::unordered_map<TKey, TValue, THash>&) {
			return true;
		}

		/// Returns \c true if container allows native value modification.
		template<typename T>
		bool AllowsNativeValueModification(const T&) {
			return IsMap(typename T::SetType());
		}

		/// Sets the dummy value of the entity with value \a value in \a set to \a dummy.
		template<typename TTraits>
		void SetDummyValue(const decltype(*TTraits::Create())& set, unsigned int value, size_t dummy) {
			// Act: find the matching entity and update its dummy value
			auto pOriginalEntity = set.find(TTraits::CreateKey("TestEntity", value));
			pOriginalEntity->Dummy = dummy;
		}

		/// Creates a set with all types of entities for batch find tests.
		template<typename TTraits>
		auto CreateSetForBatchFindTests() {
			auto pDelta = TTraits::CreateWithEntities(4);
			pDelta.commit();
			pDelta->emplace("TestEntity", 7u);
			pDelta->emplace("TestEntity", 5u);
			pDelta->emplace("TestEntity", 4u);
			pDelta->remove(TTraits::CreateKey("TestEntity", 1u));
			pDelta->remove(TTraits::CreateKey("TestEntity", 4u));
			SetDummyValue<TTraits>(*pDelta, 2, 42);
			SetDummyValue<TTraits>(*pDelta, 5, 42);
			return pDelta;
		}

		/// The expected entities in the set created by CreateSetForBatchFindTests.
		std::set<TestEntity> CreateExpectedEntitiesForBatchFindTests() {
			// Assert:
			// + 0 -> original unmodified
			// - 1 -> original removed
			// + 2 -> original copied
			// + 3 -> original unmodified
			// - 4 -> inserted removed
			// + 5 -> inserted copied
			// + 7 -> inserted unmodified
			return {
				TestEntity("TestEntity", 0),
				TestEntity("TestEntity", 2),
				TestEntity("TestEntity", 3),
				TestEntity("TestEntity", 5),
				TestEntity("TestEntity", 7)
			};
		}

		/// Const qualifies \a pObject.
		template<typename T>
		std::shared_ptr<const T> MakeConst(const std::shared_ptr<T>& pObject) {
			return pObject;
		}

		/// Asserts that the delta sizes in \a deltaWrapper have the expected values
		/// (\a expectedOriginal, \a expectedAdded, \a expectedRemoved, \a expectedCopied).
		template<typename TDeltaWrapper>
		void AssertDeltaSizes(
				TDeltaWrapper& deltaWrapper,
				size_t expectedOriginal,
				size_t expectedAdded,
				size_t expectedRemoved,
				size_t expectedCopied) {
			// Act:
			auto deltas = deltaWrapper->deltas();

			// Assert:
			CATAPULT_LOG(debug) << "size: " << deltaWrapper.originalSize()
					<< " (O " << deltaWrapper.originalSize()
					<< ", A " << deltas.Added.size()
					<< ", R " << deltas.Removed.size()
					<< ", C " << deltas.Copied.size() << ")";
			EXPECT_EQ(expectedOriginal, deltaWrapper.originalSize());
			EXPECT_EQ(expectedAdded, deltas.Added.size());
			EXPECT_EQ(expectedRemoved, deltas.Removed.size());
			EXPECT_EQ(expectedCopied, deltas.Copied.size());
		}

		/// Asserts that the delta sizes in \a delta and the original size in \a set have the expected values
		/// (\a expectedOriginal, \a expectedAdded, \a expectedRemoved, \a expectedCopied).
		template<typename TSet, typename TDelta>
		void AssertDeltaSizes(
				const TSet& set,
				const TDelta& delta,
				size_t expectedOriginal,
				size_t expectedAdded,
				size_t expectedRemoved,
				size_t expectedCopied) {
			// Act:
			auto deltas = delta.deltas();

			// Assert:
			CATAPULT_LOG(debug) << "size: " << set.size()
					<< " (O " << set.size()
					<< ", A " << deltas.Added.size()
					<< ", R " << deltas.Removed.size()
					<< ", C " << deltas.Copied.size() << ")";
			EXPECT_EQ(expectedOriginal, set.size());
			EXPECT_EQ(expectedAdded, deltas.Added.size());
			EXPECT_EQ(expectedRemoved, deltas.Removed.size());
			EXPECT_EQ(expectedCopied, deltas.Copied.size());
		}
	}

	// endregion

	// region ctor

	ALL_TRAITS_BASED_TEST(CanCreateBaseSet) {
		// Act:
		auto pSet = TTraits::Create();

		// Assert:
		TTraits::AssertContents(*pSet, typename TTraits::EntityVector());
	}

	// endregion

	// region empty

	ALL_TRAITS_BASED_TEST(EmptyReturnsTrueForEmptySet) {
		// Act:
		auto pSet = TTraits::Create();

		// Assert:
		EXPECT_TRUE(pSet->empty());
	}

	ALL_TRAITS_BASED_TEST(EmptyReturnsFalseForNonEmptySet) {
		// Act:
		auto pSet = TTraits::CreateWithEntities(3);

		// Assert:
		EXPECT_FALSE(pSet->empty());
	}

	DELTA_IMMUTABLE_TRAITS_BASED_TEST(EmptyReturnsTrueIfAllOriginalElementsAreRemoved) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		for (auto i = 0u; i < 3; i++)
			pDelta->remove(TTraits::CreateKey("TestEntity", i));

		// Assert:
		EXPECT_TRUE(pDelta->empty());
		AssertDeltaSizes(pDelta, 3, 0, 3, 0);
	}

	DELTA_IMMUTABLE_TRAITS_BASED_TEST(EmptyReturnsFalseIfANonOriginalElementWasInserted) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		for (auto i = 0u; i < 3; i++)
			pDelta->remove(TTraits::CreateKey("TestEntity", i));

		pDelta->insert(TTraits::CreateEntity("TestEntity", 123));

		// Assert:
		EXPECT_FALSE(pDelta->empty());
		AssertDeltaSizes(pDelta, 3, 1, 3, 0);
	}

	// endregion

	// region find / findIterator

	ALL_TRAITS_BASED_TEST(CanFindExistingEntity) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto entity = TTraits::CreateEntity("TestEntity", 2);

		// Act:
		auto pFoundEntity = pSet->find(TTraits::ToKey(entity));

		// Assert:
		EXPECT_EQ(3u, pSet->size());
		ASSERT_TRUE(!!pFoundEntity);
		EXPECT_EQ(*TTraits::ToPointer(entity), *pFoundEntity);
	}

	ALL_TRAITS_BASED_TEST(CanFindExistingEntityMultipleTimes) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto entity = TTraits::CreateEntity("TestEntity", 2);

		// Act:
		auto pFoundEntity1 = pSet->find(TTraits::ToKey(entity));
		auto pFoundEntity2 = pSet->find(TTraits::ToKey(entity));

		// Assert:
		EXPECT_EQ(3u, pSet->size());
		ASSERT_TRUE(!!pFoundEntity1);
		ASSERT_TRUE(!!pFoundEntity2);
		EXPECT_EQ(pFoundEntity1, pFoundEntity2);
		EXPECT_EQ(*TTraits::ToPointer(entity), *pFoundEntity1);
		EXPECT_EQ(*TTraits::ToPointer(entity), *pFoundEntity2);
	}

	ALL_TRAITS_BASED_TEST(CannotFindNonExistingEntity) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto entity1 = TTraits::CreateEntity("TestEntity", 123);
		auto entity2 = TTraits::CreateEntity("BadEntity", 2);

		// Act:
		auto pFoundEntity1 = pSet->find(TTraits::ToKey(entity1));
		auto pFoundEntity2 = pSet->find(TTraits::ToKey(entity2));

		// Assert:
		EXPECT_EQ(3u, pSet->size());
		EXPECT_FALSE(!!pFoundEntity1);
		EXPECT_FALSE(!!pFoundEntity2);
	}

	DELTA_MUTABLE_TRAITS_BASED_TEST(MutableBaseSetDeltaFindReturnsCopyForAnOriginalEntity) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto entity = TTraits::CreateEntity("TestEntity", 4);
		pDelta->insert(entity);
		pSet->commit();

		// Act:
		auto pBaseEntity = pSet->find(TTraits::ToKey(entity));
		auto pDeltaEntity = pDelta->find(TTraits::ToKey(entity));

		// Assert:
		EXPECT_NE(pBaseEntity, pDeltaEntity);
		AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 1);
	}

	DELTA_IMMUTABLE_TRAITS_BASED_TEST(ImmutableBaseSetDeltaFindReturnsOriginalEntity) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto entity = TTraits::CreateEntity("TestEntity", 4);
		pDelta->insert(entity);
		pSet->commit();

		// Act:
		auto pBaseEntity = pSet->find(TTraits::ToKey(entity));
		auto pDeltaEntity = pDelta->find(TTraits::ToKey(entity));

		// Assert:
		EXPECT_EQ(pBaseEntity, pDeltaEntity);
		AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
	}

	DELTA_MUTABLE_TRAITS_BASED_TEST(MutableBaseSetDeltaFindReturnsNonConstCopy) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto entity = TTraits::CreateEntity("TestEntity", 4);
		pDelta->insert(entity);
		pSet->commit();

		// Act:
		auto pEntity1 = pDelta->find(TTraits::ToKey(entity));
		pEntity1->Dummy = 345;
		auto pEntity2 = pDelta->find(TTraits::ToKey(entity));

		// Assert:
		EXPECT_EQ(345u, pEntity2->Dummy);

		// - if the delta contains wrapped elements, they should be mutable
		// - if the delta contains raw elements, they should be const in sets (due to the underlying set storage
		//   since sets contain const elements to prevent changing keys) but mutable in maps
		// - Dummy being changed above is mutable
		if (IsWrapped(pDelta->find(TTraits::ToKey(entity))) || AllowsNativeValueModification(*pSet))
			EXPECT_FALSE(std::is_const<decltype(Unwrap(pDelta->find(TTraits::ToKey(entity))))>());
		else
			EXPECT_TRUE(std::is_const<decltype(Unwrap(pDelta->find(TTraits::ToKey(entity))))>());

		// Sanity: the original (base) set always returns const elements
		EXPECT_TRUE(std::is_const<decltype(Unwrap(pSet->find(TTraits::ToKey(entity))))>());
		AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 1);
	}

	DELTA_IMMUTABLE_TRAITS_BASED_TEST(ImmutableBaseSetDeltaFindReturnsConstCopy) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto entity = TTraits::CreateEntity("TestEntity", 4);
		pDelta->insert(entity);
		pSet->commit();

		// Assert:
		EXPECT_TRUE(std::is_const<decltype(Unwrap(pDelta->find(TTraits::ToKey(entity))))>());

		// Sanity: the original (base) set always returns const elements
		EXPECT_TRUE(std::is_const<decltype(Unwrap(pSet->find(TTraits::ToKey(entity))))>());
		AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(BaseSetDeltaFindConstReturnsOriginalEntity) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto entity = TTraits::CreateEntity("TestEntity", 4);
		pDelta->insert(entity);
		pSet->commit();

		// Act:
		auto pBaseEntity = pSet->find(TTraits::ToKey(entity));
		auto pDeltaEntity = MakeConst(pDelta)->find(TTraits::ToKey(entity));

		// Assert:
		EXPECT_EQ(pBaseEntity, pDeltaEntity);
		AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(BaseSetDeltaFindConstReturnsConstCopy) {
		// Arrange:
		auto pSet = TTraits::CreateBase();
		auto pDelta = pSet->rebase();

		auto entity = TTraits::CreateEntity("TestEntity", 4);
		pDelta->insert(entity);
		pSet->commit();

		// Assert:
		EXPECT_TRUE(std::is_const<decltype(Unwrap(MakeConst(pDelta)->find(TTraits::ToKey(entity))))>());
		AssertDeltaSizes(*pSet, *pDelta, 1, 0, 0, 0);
	}

	namespace {
		template<typename TTraits, typename TSet>
		void AssertBatchFind(TSet& set) {
			// Assert:
			auto expectedEntities = CreateExpectedEntitiesForBatchFindTests();

			for (const auto& entity : expectedEntities)
				EXPECT_TRUE(!!set.find(TTraits::CreateKey(entity.Name, entity.Value))) << entity;

			for (auto value : { 1u, 4u, 6u, 8u, 9u })
				EXPECT_FALSE(!!set.find(TTraits::CreateKey("TestEntity", value))) << value;

			// Sanity:
			EXPECT_EQ(expectedEntities.size(), set.size());
		}
	}

	DELTA_TRAITS_BASED_TEST(BaseSetDeltaCanAccessAllElementsThroughFind) {
		// Arrange:
		auto pDelta = CreateSetForBatchFindTests<TTraits>();

		// Assert: note that (non-const) find will make copies of all elements
		AssertBatchFind<TTraits, typename decltype(pDelta)::DeltaType>(*pDelta);
		AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsEntityMutable() ? 3 : 0);
	}

	DELTA_TRAITS_BASED_TEST(BaseSetDeltaCanAccessAllElementsThroughFindConst) {
		// Arrange:
		auto pDelta = CreateSetForBatchFindTests<TTraits>();

		// Assert:
		AssertBatchFind<TTraits, const typename decltype(pDelta)::DeltaType>(*pDelta);
		AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsEntityMutable() ? 1 : 0);
	}

	NON_DELTA_TRAITS_BASED_TEST(FindIteratorReturnsIteratorToEntityIfEntityExists) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto entity = TTraits::CreateEntity("TestEntity", 1);

		// Act:
		auto it = pSet->findIterator(TTraits::ToKey(entity));

		// Assert:
		EXPECT_EQ(*TTraits::ToPointer(entity), *TTraits::ToPointerFromStorage(*it));
	}

	NON_DELTA_TRAITS_BASED_TEST(FindIteratorReturnsCendIfEntityDoesNotExist) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto entity = TTraits::CreateEntity("TestEntity", 4);

		// Act:
		auto it = pSet->findIterator(TTraits::ToKey(entity));

		// Assert:
		EXPECT_EQ(pSet->cend(), it);
	}

	// endregion

	// region contains

	ALL_TRAITS_BASED_TEST(ContainsReturnsTrueForExistingEntity) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto entity = TTraits::CreateEntity("TestEntity", 2);

		// Act:
		auto found1 = pSet->contains(TTraits::ToKey(entity));

		// Assert:
		EXPECT_TRUE(found1);
	}

	ALL_TRAITS_BASED_TEST(ContainsReturnsFalseForNonExistingEntity) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto entity1 = TTraits::CreateEntity("TestEntity", 123);
		auto entity2 = TTraits::CreateEntity("BadEntity", 2);

		// Act:
		auto found1 = pSet->contains(TTraits::ToKey(entity1));
		auto found2 = pSet->contains(TTraits::ToKey(entity2));

		// Assert:
		EXPECT_FALSE(found1);
		EXPECT_FALSE(found2);
	}

	// end region

	// region BaseSet iterators and set order

	namespace {
		template<typename TBaseSet>
		void AssertConstIterator(const TBaseSet& set) {
			// Assert: the unwrapped set iterator entity pointer points to a const entity
			auto iter = set.cbegin();
			EXPECT_TRUE(std::is_const<decltype(Unwrap(iter.operator->()))>());
		}
	}

	namespace {
		template<typename TTraits>
		void AssertBeginEndIteratorConsistency(const decltype(*TTraits::Create())& set) {
			// Sanity: ensure cbegin and cend iterators are only equal when the set is empty
			if (set.empty())
				EXPECT_EQ(set.cbegin(), set.cend());
			else
				EXPECT_NE(set.cbegin(), set.cend());
		}

		template<typename TTraits>
		std::set<TestEntity> ExtractEntities(const decltype(*TTraits::Create())& set) {
			// Act: iterate over the set and extract all entities
			std::set<TestEntity> entities;
			size_t numIteratedEntities = 0;
			for (auto iter = set.cbegin(); set.cend() != iter; ++iter) {
				entities.insert(*TTraits::ToPointerFromStorage(*iter));
				++numIteratedEntities;
			}

			// Sanity: iterators are consistent with size and there are no duplicate entities in the set
			AssertBeginEndIteratorConsistency<TTraits>(set);
			EXPECT_EQ(set.size(), numIteratedEntities);
			return entities;
		}

		template<typename TTraits>
		void AssertIteration(const decltype(*TTraits::Create())& set, const std::set<TestEntity>& expectedEntities) {
			// Act:
			auto actualEntities = ExtractEntities<TTraits>(set);

			// Assert:
			EXPECT_EQ(expectedEntities, actualEntities);

			// Sanity: the iterator elements are const
			AssertConstIterator(set);
		}
	}

	ALL_TRAITS_BASED_TEST(CanIterateThroughEmptySetWithConstIterator) {
		// Assert:
		AssertIteration<TTraits>(*TTraits::Create(), {});
	}

	ALL_TRAITS_BASED_TEST(CanIterateThroughSingleValueSetWithConstIterator) {
		// Assert:
		AssertIteration<TTraits>(*TTraits::CreateWithEntities(1), { TestEntity("TestEntity", 0) });
	}

	ALL_TRAITS_BASED_TEST(CanIterateThroughMultiValueSetWithConstIterator) {
		// Assert:
		std::set<TestEntity> expectedEntities{
			TestEntity("TestEntity", 0),
			TestEntity("TestEntity", 1),
			TestEntity("TestEntity", 2)
		};
		AssertIteration<TTraits>(*TTraits::CreateWithEntities(3), expectedEntities);
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationIncludesOriginalElements) {
		// Arrange: commit so that all entities become original elements
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();

		// Act:
		auto actualEntities = ExtractEntities<TTraits>(*pDelta);

		// Assert:
		std::set<TestEntity> expectedEntities{
			TestEntity("TestEntity", 0),
			TestEntity("TestEntity", 1),
			TestEntity("TestEntity", 2)
		};
		EXPECT_EQ(expectedEntities, actualEntities);
		AssertDeltaSizes(pDelta, 3, 0, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationCanIterateOnlyInsertedElements) {
		// Arrange:
		auto pDelta = TTraits::Create();
		pDelta->emplace("TestEntity", 7u);
		pDelta->emplace("TestEntity", 4u);

		// Act:
		auto actualEntities = ExtractEntities<TTraits>(*pDelta);

		// Assert:
		std::set<TestEntity> expectedEntities{
			TestEntity("TestEntity", 4),
			TestEntity("TestEntity", 7)
		};
		EXPECT_EQ(expectedEntities, actualEntities);
		AssertDeltaSizes(pDelta, 0, 2, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationIncludesInsertedElements) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		pDelta->emplace("TestEntity", 7u);
		pDelta->emplace("TestEntity", 4u);

		// Act:
		auto actualEntities = ExtractEntities<TTraits>(*pDelta);

		// Assert:
		std::set<TestEntity> expectedEntities{
			TestEntity("TestEntity", 0),
			TestEntity("TestEntity", 1),
			TestEntity("TestEntity", 2),
			TestEntity("TestEntity", 4),
			TestEntity("TestEntity", 7)
		};
		EXPECT_EQ(expectedEntities, actualEntities);
		AssertDeltaSizes(pDelta, 3, 2, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationExcludesRemovedElements) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		pDelta->emplace("TestEntity", 7u);
		pDelta->emplace("TestEntity", 4u);
		pDelta->remove(TTraits::CreateKey("TestEntity", 1u));
		pDelta->remove(TTraits::CreateKey("TestEntity", 4u));

		// Act:
		auto actualEntities = ExtractEntities<TTraits>(*pDelta);

		// Assert:
		std::set<TestEntity> expectedEntities{
			TestEntity("TestEntity", 0),
			TestEntity("TestEntity", 2),
			TestEntity("TestEntity", 7)
		};
		EXPECT_EQ(expectedEntities, actualEntities);
		AssertDeltaSizes(pDelta, 3, 1, 1, 0);
	}

	namespace {
		template<typename TTraits>
		std::set<size_t> ExtractDummyValues(const decltype(*TTraits::Create())& set) {
			std::set<size_t> dummyValues;
			size_t numIteratedEntities = 0;
			for (auto iter = set.cbegin(); set.cend() != iter; ++iter) {
				dummyValues.insert(TTraits::ToPointerFromStorage(*iter)->Dummy);
				++numIteratedEntities;
			}

			// Sanity: iterators are consistent with size and there are no duplicate entities in the set
			AssertBeginEndIteratorConsistency<TTraits>(set);
			EXPECT_EQ(set.size(), numIteratedEntities);
			return dummyValues;
		}
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationIncludesCopiedElements) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		SetDummyValue<TTraits>(*pDelta, 0, 0);
		SetDummyValue<TTraits>(*pDelta, 1, 1);
		SetDummyValue<TTraits>(*pDelta, 2, 2);
		pDelta.commit(); // the 3 entities are now original entities with dummy values set

		// - change the dummy value for one entity
		SetDummyValue<TTraits>(*pDelta, 1, 42);

		// Act:
		auto actualDummyValues = ExtractDummyValues<TTraits>(*pDelta);

		// Assert: iterating should pick up the new dummy value
		//         (in the case of mutable, the copied entity will be returned in place of the original entity)
		//         (in the case of immutable, the original entity with the changed value will be returned)
		std::set<size_t> expectedDummyValues{ 0, 42, 2 };
		EXPECT_EQ(expectedDummyValues, actualDummyValues);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsEntityMutable() ? 1 : 0);
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationCanIterateOnlyCopiedElements) {
		// Arrange:
		constexpr auto Num_Entities = 3u;
		auto pDelta = TTraits::CreateWithEntities(Num_Entities);
		for (auto i = 0u; i < Num_Entities; ++i)
			SetDummyValue<TTraits>(*pDelta, i, i);
		pDelta.commit(); // the 3 entities are now original entities with dummy values set

		// - make copies of all the entities
		for (auto i = 0u; i < Num_Entities; ++i)
			SetDummyValue<TTraits>(*pDelta, i, i + 7);

		// Act:
		auto actualDummyValues = ExtractDummyValues<TTraits>(*pDelta);

		// Assert: iterating should pick up the new dummy values
		std::set<size_t> expectedDummyValues{ 7, 8, 9 };
		EXPECT_EQ(expectedDummyValues, actualDummyValues);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsEntityMutable() ? 3 : 0);
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationDoesNotReiterateNewlyCopiedElements) {
		// Arrange: create a set with three original entities with dummy values
		auto pDelta = TTraits::CreateWithEntities(3);
		SetDummyValue<TTraits>(*pDelta, 0, 0);
		SetDummyValue<TTraits>(*pDelta, 1, 1);
		SetDummyValue<TTraits>(*pDelta, 2, 2);
		pDelta.commit(); // the 3 entities are now original entities with dummy values set

		// - add a fourth (uncommitted) entity with a dummy value
		pDelta->emplace("TestEntity", 3u);
		SetDummyValue<TTraits>(*pDelta, 3, 3);

		// Act: iterate
		std::set<size_t> dummyValues;
		size_t numIteratedEntities = 0;
		for (auto iter = pDelta->cbegin(); pDelta->cend() != iter; ++iter) {
			auto pCurrentEntity = TTraits::ToPointerFromStorage(*iter);
			auto dummyValue = pCurrentEntity->Dummy;
			dummyValues.insert(dummyValue);
			++numIteratedEntities;

			// - change the dummy value of the middle elements
			if (1 > dummyValue || dummyValue > 2) continue;
			auto pEntityCopy = pDelta->find(TTraits::CreateKey(pCurrentEntity->Name, pCurrentEntity->Value));
			pEntityCopy->Dummy = dummyValue + 40;
		}

		// Assert: initial iteration should pick up the old dummy values
		std::set<size_t> expectedDummyValues{ 0, 1, 2, 3 };
		EXPECT_EQ(expectedDummyValues.size(), numIteratedEntities);
		EXPECT_EQ(expectedDummyValues, dummyValues);

		// Sanity: reiterating should pick up the new dummy values
		expectedDummyValues = { 0, 41, 42, 3 };
		EXPECT_EQ(expectedDummyValues, ExtractDummyValues<TTraits>(*pDelta));
		AssertDeltaSizes(pDelta, 3, 1, 0, TTraits::IsEntityMutable() ? 2 : 0);
	}

	DELTA_TRAITS_BASED_TEST(DeltaCannotDereferenceAtEnd) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		auto iter = pDelta->cend();

		// Act + Assert:
		EXPECT_THROW(*iter, catapult_out_of_range);
		EXPECT_THROW(iter.operator->(), catapult_out_of_range);
	}

	DELTA_TRAITS_BASED_TEST(DeltaCannotAdvancePrefixIteratorBeyondEnd) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		auto iter = pDelta->cend();

		// Act + Assert:
		EXPECT_THROW(++iter, catapult_out_of_range);
		EXPECT_THROW(++iter, catapult_out_of_range);
	}

	DELTA_TRAITS_BASED_TEST(DeltaCannotAdvancePostfixIteratorBeyondEnd) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		auto iter = pDelta->cend();

		// Act + Assert:
		EXPECT_THROW(iter++, catapult_out_of_range);
		EXPECT_THROW(iter++, catapult_out_of_range);
	}

	DELTA_TRAITS_BASED_TEST(DeltaBeginEndIteratorsBasedOnSameContainerAreEqual) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);

		// Act + Assert:
		EXPECT_EQ(pDelta->cbegin(), pDelta->cbegin());
		EXPECT_EQ(pDelta->cend(), pDelta->cend());
		EXPECT_NE(pDelta->cbegin(), pDelta->cend());
	}

	DELTA_TRAITS_BASED_TEST(DeltaBeginEndIteratorsBasedOnDifferentContainerAreNotEqual) {
		// Arrange:
		auto pDelta1 = TTraits::CreateWithEntities(3);
		auto pDelta2 = TTraits::CreateWithEntities(3);

		// Act + Assert:
		EXPECT_NE(pDelta1->cbegin(), pDelta2->cbegin());
		EXPECT_NE(pDelta1->cend(), pDelta2->cend());
	}

	namespace {
		template<typename TTraits>
		void AssertDeltaIteration(const std::function<void (decltype(TTraits::Create()->cbegin())&)>& increment) {
			// Arrange:
			auto pDelta = CreateSetForBatchFindTests<TTraits>();

			// Act:
			std::set<TestEntity> actualEntities;
			size_t numIteratedEntities = 0;
			for (auto iter = pDelta->cbegin(); pDelta->cend() != iter; increment(iter)) {
				actualEntities.insert(*TTraits::ToPointerFromStorage(*iter));
				++numIteratedEntities;

				// Sanity: * and -> return same entity
				EXPECT_EQ(TTraits::ToPointerFromStorage(*iter), TTraits::ToPointerFromStorage(*(iter.operator->())));
			}

			// Assert:
			EXPECT_EQ(pDelta->size(), numIteratedEntities);
			auto expectedEntities = CreateExpectedEntitiesForBatchFindTests();
			EXPECT_EQ(expectedEntities, actualEntities);

			// Sanity: value_type should be const
			EXPECT_TRUE(std::is_const<typename decltype(pDelta->cbegin())::value_type>());
			AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsEntityMutable() ? 1 : 0);
		}
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationSupportsPostfixIterator) {
		AssertDeltaIteration<TTraits>([](auto& iter) { iter++; });
	}

	DELTA_TRAITS_BASED_TEST(DeltaIterationSupportsPrefixIterator) {
		// Assert:
		AssertDeltaIteration<TTraits>([](auto& iter) { ++iter; });
	}

	// endregion

	// region insert

	DELTA_TRAITS_BASED_TEST(CanInsertEntity) {
		// Arrange:
		auto pDelta = TTraits::Create();
		auto entity = TTraits::CreateEntity("MyTestEntity", 321);
		auto expectedEntities = typename TTraits::EntityVector{ entity };

		// Act:
		pDelta->insert(entity);

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 0, 1, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(CanInsertWithSuppliedParameters) {
		// Arrange:
		auto pDelta = TTraits::Create();
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("MyTestEntity", 0),
			TTraits::CreateEntity("AnotherTestEntity", 123)
		};

		// Act:
		pDelta->emplace("MyTestEntity");
		pDelta->emplace("AnotherTestEntity", 123u);

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 0, 2, 0, 0);
	}

	namespace {
		template<typename TTraits>
		void AssertCopyInsertNewEntity(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::Create();
			auto entity = TTraits::CreateEntity("TestEntity", 4);
			TTraits::ToPointer(entity)->Dummy = 123;
			pDelta->insert(entity); // pEntity is not an original entity

			// Act:
			auto identicalEntity = TTraits::CreateEntity("TestEntity", 4);
			for (auto i = 1u; i < 10; ++i) {
				TTraits::ToPointer(identicalEntity)->Dummy = i;
				pDelta->insert(identicalEntity);
			}

			auto pCurrent = pDelta->find(TTraits::ToKey(entity));

			// Assert:
			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 0, 1, 0, 0);
		}
	}

	DELTA_MUTABLE_TRAITS_BASED_TEST(MutableBaseSetDeltaInsertUpdatesKnownEntity) {
		// Assert:
		AssertCopyInsertNewEntity<TTraits>(9u);
	}

	DELTA_IMMUTABLE_TRAITS_BASED_TEST(ImmutableBaseSetDeltaInsertDoesNotUpdateKnownEntity) {
		// Assert:
		AssertCopyInsertNewEntity<TTraits>(123u);
	}

	DELTA_TRAITS_BASED_TEST(BaseSetDeltaSupportsReinsertingExistingEntity) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();

		// Act: find the matching entity, update its dummy value, and reinsert it
		auto pOriginalEntity = pDelta->find(TTraits::CreateKey("TestEntity", 1));
		pOriginalEntity->Dummy = 777;
		pDelta->insert(TTraits::ToSetEntity(pOriginalEntity));

		// - get the updated entity
		auto pUpdatedEntity = pDelta->find(TTraits::CreateKey("TestEntity", 1));

		// Assert: it has been reinserted with the updated dummy value
		EXPECT_EQ(777u, pUpdatedEntity->Dummy);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsEntityMutable() ? 1 : 0);
	}

	namespace {
		template<typename TTraits>
		void AssertCopyInsertOriginalEntity(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::CreateWithEntities(3);
			auto pOriginalEntity = pDelta->find(TTraits::CreateKey("TestEntity", 2));
			pOriginalEntity->Dummy = 123;
			pDelta->insert(TTraits::ToSetEntity(pOriginalEntity));
			pDelta.commit(); // the 3 entities are now original entities

			// find will insert a copy of the original entity into the set of copies
			pDelta->find(TTraits::CreateKey("TestEntity", 2));

			// entity is a clone of the above copied original entity, but not the same object instance
			auto entity = TTraits::CreateEntity("TestEntity", 2);

			// Act:
			for (auto i = 1u; i < 10; ++i) {
				TTraits::ToPointer(entity)->Dummy = i;
				pDelta->insert(entity);
			}

			auto pCurrent = pDelta->find(TTraits::CreateKey("TestEntity", 2));

			// Assert:
			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsEntityMutable() ? 1 : 0);
		}
	}

	DELTA_MUTABLE_TRAITS_BASED_TEST(MutableBaseSetDeltaInsertUpdatesCopiedOriginalEntity) {
		// Assert:
		AssertCopyInsertOriginalEntity<TTraits>(9u);
	}

	DELTA_IMMUTABLE_TRAITS_BASED_TEST(ImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalEntity) {
		// Assert:
		AssertCopyInsertOriginalEntity<TTraits>(123u);
	}

	namespace {
		template<typename TTraits>
		void AssertCopyInsertOriginalEntityAfterRemoval(uint32_t expectedValue) {
			// Arrange:
			auto pDelta = TTraits::CreateWithEntities(3);
			auto pOriginalEntity = pDelta->find(TTraits::CreateKey("TestEntity", 2));
			pOriginalEntity->Dummy = 123;
			pDelta->insert(TTraits::ToSetEntity(pOriginalEntity));
			pDelta.commit(); // the 3 entities are now original entities

			// - insert an (original) entity
			auto entity = TTraits::CreateEntity("TestEntity", 2);
			TTraits::ToPointer(entity)->Dummy = 3;
			pDelta->insert(entity);

			// - remove the copy
			pDelta->remove(TTraits::ToKey(entity));

			// - reinsert the copy
			auto entity2 = TTraits::CreateEntity("TestEntity", 2);
			TTraits::ToPointer(entity2)->Dummy = 9;
			pDelta->insert(entity2);

			// Act: get the current element
			auto pCurrent = pDelta->find(TTraits::CreateKey("TestEntity", 2));

			// Assert:
			EXPECT_EQ(expectedValue, pCurrent->Dummy);
			AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsEntityMutable() ? 1 : 0);
		}
	}

	DELTA_MUTABLE_TRAITS_BASED_TEST(MutableBaseSetDeltaInsertUpdatesCopiedOriginalEntityAfterRemoval) {
		// Assert:
		AssertCopyInsertOriginalEntityAfterRemoval<TTraits>(9u);
	}

	DELTA_IMMUTABLE_TRAITS_BASED_TEST(ImmutableBaseSetDeltaInsertDoesNotUpdateCopiedOriginalEntityAfterRemoval) {
		// Assert:
		AssertCopyInsertOriginalEntityAfterRemoval<TTraits>(123u);
	}

	NON_DELTA_TRAITS_BASED_TEST(InsertDoesNotChangeOriginalBaseSet) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto pDelta = pBaseSet->rebase();
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		pDelta->emplace("MyEntity", 123u);

		// Assert:
		EXPECT_EQ(4u, pDelta->size());
		TTraits::AssertContents(*pBaseSet, expectedEntities);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 1, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(InsertOfSameAlreadyInsertedEntityIsNullOperation) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		for (auto i = 0u; i < 10; ++i)
			pDelta->emplace("TestEntity", 1u);

		// Assert:
		EXPECT_EQ(3u, pDelta->size());
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 0, 3, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(CanInsertRemovedEntity) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		auto expectedEntities = TTraits::CreateEntities(3);
		pDelta->remove(TTraits::CreateKey("TestEntity", 1));

		// sanity check
		EXPECT_EQ(2u, pDelta->size());
		EXPECT_FALSE(!!pDelta->find(TTraits::CreateKey("TestEntity", 1)));

		// Act:
		pDelta->emplace("TestEntity", 1u);

		// Assert:
		EXPECT_EQ(3u, pDelta->size());
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 3, 0, 0, TTraits::IsEntityMutable() ? 1 : 0);
	}

	// endregion

	// region remove

	DELTA_TRAITS_BASED_TEST(CanRemoveExistingEntity) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		auto entity = TTraits::CreateEntity("TestEntity", 1);
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 0),
			TTraits::CreateEntity("TestEntity", 2)
		};

		// Act:
		pDelta->remove(TTraits::ToKey(entity));

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 3, 0, 1, 0);
	}

	DELTA_TRAITS_BASED_TEST(CanSubsequentlyRemoveExistingEntities) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(5);
		pDelta.commit();
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 0),
			TTraits::CreateEntity("TestEntity", 2)
		};

		// Act:
		pDelta->remove(TTraits::CreateKey("TestEntity", 1));
		pDelta->remove(TTraits::CreateKey("TestEntity", 3));
		pDelta->remove(TTraits::CreateKey("TestEntity", 4));

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 5, 0, 3, 0);
	}

	DELTA_TRAITS_BASED_TEST(CanRemovePreviouslyInsertedEntity) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		auto expectedEntities = TTraits::CreateEntities(3);
		pDelta->insert(TTraits::CreateEntity("MyTestEntity", 234));

		// Sanity check
		EXPECT_EQ(4u, pDelta->size());
		EXPECT_TRUE(!!pDelta->find(TTraits::CreateKey("MyTestEntity", 234)));

		// Act:
		pDelta->remove(TTraits::CreateKey("MyTestEntity", 234));

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 3, 0, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(CanRemoveMutatedEntity) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		auto pEntity = pDelta->find(TTraits::CreateKey("TestEntity", 1));
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 0),
			TTraits::CreateEntity("TestEntity", 2)
		};

		// Act:
		pEntity->Dummy = 456;
		pDelta->remove(TTraits::ToKey(TTraits::ToSetEntity(pEntity)));

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 3, 0, 1, 0);
	}

	DELTA_TRAITS_BASED_TEST(RemovingNonExistingEntityDoesNotRemoveAnyEntity) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		auto entity = TTraits::CreateEntity("TestEntity", 4);
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		pDelta->remove(TTraits::ToKey(entity));

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 3, 0, 0, 0);
	}

	DELTA_TRAITS_BASED_TEST(RemoveOfAlreadyRemovedEntityIsNullOperation) {
		// Arrange:
		auto pDelta = TTraits::CreateWithEntities(3);
		pDelta.commit();
		pDelta->remove(TTraits::CreateKey("TestEntity", 1));
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 0),
			TTraits::CreateEntity("TestEntity", 2)
		};

		// Sanity check
		EXPECT_EQ(2u, pDelta->size());
		EXPECT_FALSE(!!pDelta->find(TTraits::CreateKey("TestEntity", 1)));

		// Act:
		for (auto i = 0u; i < 10; ++i)
			pDelta->remove(TTraits::CreateKey("TestEntity", 1));

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(pDelta, 3, 0, 1, 0);
	}

	NON_DELTA_TRAITS_BASED_TEST(RemoveDoesNotChangeOriginalBaseSet) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto pDelta = pBaseSet->rebase();
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		pDelta->remove(TTraits::CreateKey("TestEntity", 2));

		// Assert:
		EXPECT_EQ(2u, pDelta->size());
		TTraits::AssertContents(*pBaseSet, expectedEntities);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 1, 0);
	}

	// endregion

	// region reset

	DELTA_TRAITS_BASED_TEST(BaseSetDeltaResetClearsAllPendingChanges) {
		// Arrange:
		auto pDelta = CreateSetForBatchFindTests<TTraits>();

		// Sanity:
		AssertDeltaSizes(pDelta, 4, 2, 1, TTraits::IsEntityMutable() ? 1 : 0);

		// Act:
		pDelta->reset();

		// Assert:
		AssertDeltaSizes(pDelta, 4, 0, 0, 0);
	}

	// endregion

	// region rebase / rebaseDetached

	NON_DELTA_TRAITS_BASED_TEST(RebaseCreatesDeltaAroundSuppliedEntities) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		auto pDelta = pBaseSet->rebase();

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
	}

	NON_DELTA_TRAITS_BASED_TEST(RebaseAllowsOnlyOneAttachedDeltaAtATime) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);
		{
			auto pDelta = pBaseSet->rebase();

			// Act + Assert:
			EXPECT_THROW(pBaseSet->rebase(), catapult_runtime_error);
		}

		// Act: delta went out of scope, another delta is allowed
		auto pDelta = pBaseSet->rebase();
		TTraits::AssertContents(*pDelta, expectedEntities);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
	}

	NON_DELTA_TRAITS_BASED_TEST(RebaseDetachedCreatesDeltaAroundSuppliedEntities) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		auto pDetachedDelta = pBaseSet->rebaseDetached();

		// Assert:
		TTraits::AssertContents(*pDetachedDelta, expectedEntities);
		AssertDeltaSizes(*pBaseSet, *pDetachedDelta, 3, 0, 0, 0);
	}

	NON_DELTA_TRAITS_BASED_TEST(RebaseDetachedAllowsManyDeltas) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);
		std::vector<decltype(pBaseSet->rebaseDetached())> deltas;

		// Act:
		for (auto i = 0u; i < 10; ++i)
			deltas.push_back(pBaseSet->rebaseDetached());

		// Assert:
		for (const auto& pDetachedDelta : deltas) {
			TTraits::AssertContents(*pDetachedDelta, expectedEntities);
			AssertDeltaSizes(*pBaseSet, *pDetachedDelta, 3, 0, 0, 0);
		}
	}

	// endregion

	// region commit

	NON_DELTA_TRAITS_BASED_TEST(CannotCommitWhenThereAreNoPendingAttachedDeltas) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);

		// Act:
		EXPECT_THROW(pBaseSet->commit(), catapult_runtime_error);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitThrowsIfOnlyDetachedDeltasAreOutstanding) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDetachedDelta = pSet->rebaseDetached();

		// Assert:
		EXPECT_THROW(pSet->commit(), catapult_runtime_error);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitCommitsToOriginalEntities) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto pDelta = pBaseSet->rebase();
		pDelta->emplace("MyTestEntity", 123u);
		pDelta->remove(TTraits::CreateKey("TestEntity", 0));
		pDelta->remove(TTraits::CreateKey("TestEntity", 2));
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 1),
			TTraits::CreateEntity("MyTestEntity", 123),
		};

		// Act:
		pBaseSet->commit();

		// Assert:
		TTraits::AssertContents(*pBaseSet, expectedEntities);
		AssertDeltaSizes(*pBaseSet, *pDelta, 2, 0, 0, 0);
	}

	NON_DELTA_MUTABLE_TRAITS_BASED_TEST(CommitReflectsChangesOnOriginalEntities) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto pDelta = pBaseSet->rebase();
		auto pEntity = pDelta->find(TTraits::CreateKey("TestEntity", 1));

		// Act:
		pEntity->Dummy = 123;
		pBaseSet->commit();
		auto pEntityAfterCommit = pBaseSet->find(TTraits::CreateKey("TestEntity", 1));

			// Assert:
		EXPECT_EQ(123u, pEntityAfterCommit->Dummy);
		AssertDeltaSizes(*pBaseSet, *pDelta, 3, 0, 0, 0);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitIsIdempotent) {
		// Arrange:
		auto pBaseSet = TTraits::CreateWithEntities(3);
		auto pDelta = pBaseSet->rebase();
		pDelta->emplace("MyTestEntity", 123u);
		pDelta->remove(TTraits::CreateKey("TestEntity", 0));
		pDelta->remove(TTraits::CreateKey("TestEntity", 2));
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 1),
			TTraits::CreateEntity("MyTestEntity", 123),
		};

		// Act:
		for (auto i = 0u; i < 5u; ++i)
			pBaseSet->commit();

		// Assert:
		TTraits::AssertContents(*pBaseSet, expectedEntities);
		AssertDeltaSizes(*pBaseSet, *pDelta, 2, 0, 0, 0);
	}

	// endregion
}}
