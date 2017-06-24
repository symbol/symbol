#include "catapult/deltaset/OrderedSet.h"
#include "catapult/exceptions.h"
#include "tests/catapult/deltaset/utils/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		template<typename TSet>
		void CommitWithoutPruning(TSet& set) {
			using BoundaryType = typename std::remove_const<typename TSet::EntityType>::type;
			set.commit(PruningBoundary<BoundaryType>());
		}

		template<typename TEntityTraits>
		struct OrderedSetTraits {
			using Type = OrderedSet<TEntityTraits>;
			using DeltaType = OrderedSetDelta<TEntityTraits>;
			using EntityType = typename TEntityTraits::EntityType;
			using SetTraits = typename DeltaType::SetTraits;

			static auto Create() {
				return CreateOrderedSet<TEntityTraits>();
			}

			static void Commit(Type& set) {
				CommitWithoutPruning(set);
			}
		};

		template<typename TMutabilityTraits>
		using OrderedSetTypeTraits = OrderedSetTraits<TMutabilityTraits>;

		using MutableTraits = OrderedSetTypeTraits<MutableTypeTraits<MutableTestEntity>>;
		using MutablePointerTraits = OrderedSetTypeTraits<MutableTypeTraits<std::shared_ptr<MutableTestEntity>>>;
		using ImmutableTraits = OrderedSetTypeTraits<ImmutableTypeTraits<const ImmutableTestEntity>>;
		using ImmutablePointerTraits =
				OrderedSetTypeTraits<ImmutableTypeTraits<std::shared_ptr<const ImmutableTestEntity>>>;
	}

#define DEFINE_SIMPLE_TEST(TEST_NAME, TYPES) \
	template<typename TTraits> void TRAITS_TEST_NAME(OrderedSetTests, TEST_NAME)(); \
	TYPES(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(OrderedSetTests, TEST_NAME)()

#define REGISTER_TEST(TEST_NAME, TRAITS_TYPE, TRAITS_NAME) \
	TEST(OrderedSetTests, TEST_NAME##_##TRAITS_NAME) { TRAITS_TEST_NAME(OrderedSetTests, TEST_NAME)<TRAITS_TYPE>(); }

#define REGISTER_DELTA_TESTS(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, DeltaTraits<MutablePointerTraits>, DeltaMutablePointer); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<ImmutablePointerTraits>, DeltaImmutablePointer); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<MutableTraits>, DeltaMutable); \
	REGISTER_TEST(TEST_NAME, DeltaTraits<ImmutableTraits>, DeltaImmutable); \

#define REGISTER_NON_DELTA_TESTS(TEST_NAME) \
	REGISTER_TEST(TEST_NAME, BaseTraits<MutablePointerTraits>, BaseMutablePointer); \
	REGISTER_TEST(TEST_NAME, BaseTraits<ImmutablePointerTraits>, BaseImmutablePointer); \
	REGISTER_TEST(TEST_NAME, BaseTraits<MutableTraits>, BaseMutable); \
	REGISTER_TEST(TEST_NAME, BaseTraits<ImmutableTraits>, BaseImmutable); \

#define REGISTER_ALL_TESTS(TEST_NAME) \
	REGISTER_DELTA_TESTS(TEST_NAME) \
	REGISTER_NON_DELTA_TESTS(TEST_NAME) \

#define NON_DELTA_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_NON_DELTA_TESTS)
#define ALL_TRAITS_BASED_TEST(TEST_NAME) DEFINE_SIMPLE_TEST(TEST_NAME, REGISTER_ALL_TESTS)

	// region ctor

	ALL_TRAITS_BASED_TEST(CanCreateOrderedSet) {
		// Act:
		auto pSet = TTraits::Create();

		// Assert:
		TTraits::AssertContents(*pSet, typename TTraits::EntityVector());
	}

	// endregion

	// region rebase

	NON_DELTA_TRAITS_BASED_TEST(RebaseCreatesOrderedSetAroundSuppliedEntities) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		auto pDelta = pSet->rebase();

		// Assert:
		TTraits::AssertContents(*pDelta, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(RebaseAllowsOnlyOneAttachedDeltaAtATime) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);
		{
			auto pDelta = pSet->rebase();

			// Act + Assert:
			EXPECT_THROW(pSet->rebase(), catapult_runtime_error);
		}

		// Act: delta went out of scope, another delta is allowed
		auto pDelta = pSet->rebase();
		TTraits::AssertContents(*pDelta, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(RebaseDetachedCreatesOrderedSetAroundSuppliedEntities) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);

		// Act:
		auto pDetachedDelta = pSet->rebaseDetached();

		// Assert:
		TTraits::AssertContents(*pDetachedDelta, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(RebaseDetachedAllowsManyDeltas) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto expectedEntities = TTraits::CreateEntities(3);
		std::vector<decltype(pSet->rebaseDetached())> deltas;

		// Act:
		for (auto i = 0u; i < 10; ++i)
			deltas.push_back(pSet->rebaseDetached());

		// Assert:
		for (const auto& pDelta : deltas)
			TTraits::AssertContents(*pDelta, expectedEntities);
	}

	// endregion

	// region commit

	NON_DELTA_TRAITS_BASED_TEST(CommitWithUnsetPruningBoundaryCommitsToOriginalEntitiesWithoutPruning) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(3);
		auto pDelta = pSet->rebase();
		pDelta->emplace("MyTestEntity", 123u);
		pDelta->remove(TTraits::CreateEntity("TestEntity", 0));
		pDelta->remove(TTraits::CreateEntity("TestEntity", 2));
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 1),
			TTraits::CreateEntity("MyTestEntity", 123),
		};

		// Act:
		CommitWithoutPruning(*pSet);

		// Assert:
		TTraits::AssertContents(*pSet, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitPrunesEntitiesPreviousToPruningBoundary) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(5);
		auto pDelta = pSet->rebase();
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 3),
			TTraits::CreateEntity("TestEntity", 4),
		};
		auto pruningBoundary = TTraits::CreateEntity("TestEntity", 3);

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitIsNullOperationIfPruningBoundaryIsEqualToFirstSetElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(5);
		auto pDelta = pSet->rebase();
		pDelta->remove(TTraits::CreateEntity("TestEntity", 0));
		pDelta->remove(TTraits::CreateEntity("TestEntity", 1));
		CommitWithoutPruning(*pSet);
		auto pruningBoundary = TTraits::CreateEntity("TestEntity", 2);
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 2),
			TTraits::CreateEntity("TestEntity", 3),
			TTraits::CreateEntity("TestEntity", 4),
		};

		// Sanity check:
		auto firstEntity = *(pSet->cbegin());
		EXPECT_EQ(*TTraits::ToPointer(firstEntity), *TTraits::ToPointer(pruningBoundary));

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitIsNullOperationIfPruningBoundaryIsSmallerThanFirstSetElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(5);
		auto pDelta = pSet->rebase();
		pDelta->remove(TTraits::CreateEntity("TestEntity", 0));
		pDelta->remove(TTraits::CreateEntity("TestEntity", 1));
		CommitWithoutPruning(*pSet);
		auto pruningBoundary = TTraits::CreateEntity("TestEntity", 1);
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 2),
			TTraits::CreateEntity("TestEntity", 3),
			TTraits::CreateEntity("TestEntity", 4),
		};

		// Sanity check:
		auto firstEntity = *(pSet->cbegin());
		EXPECT_TRUE(*TTraits::ToPointer(pruningBoundary) < *TTraits::ToPointer(firstEntity));

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitPruningBoundaryDoesNotNeedToBeInSet) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(5);
		auto pDelta = pSet->rebase();
		auto pruningBoundary = TTraits::CreateEntity("TestEntity", 2);
		pDelta->remove(pruningBoundary);
		CommitWithoutPruning(*pSet);
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 3),
			TTraits::CreateEntity("TestEntity", 4),
		};

		// Sanity check:
		EXPECT_FALSE(!!pSet->find(pruningBoundary));

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitIsIdempotent) {
		// Arrange:
		auto pSet = TTraits::CreateWithEntities(5);
		auto pDelta = pSet->rebase();
		auto pruningBoundary = TTraits::CreateEntity("TestEntity", 3);
		auto expectedEntities = typename TTraits::EntityVector{
			TTraits::CreateEntity("TestEntity", 3),
			TTraits::CreateEntity("TestEntity", 4),
		};

		// Act:
		for (auto i = 0u; i < 10u; ++i)
			pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedEntities);
	}

	NON_DELTA_TRAITS_BASED_TEST(CommitThrowsIfOnlyDetachedDeltasAreOutstanding) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDetachedDelta = pSet->rebaseDetached();

		// Assert:
		EXPECT_THROW(CommitWithoutPruning(*pSet), catapult_runtime_error);
	}

	// endregion
}}
