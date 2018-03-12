#include "catapult/deltaset/OrderedSet.h"
#include "catapult/exceptions.h"
#include "tests/catapult/deltaset/test/BaseSetTestsInclude.h"

namespace catapult { namespace deltaset {

	namespace {
		template<typename TSet>
		void CommitWithoutPruning(TSet& set) {
			using BoundaryType = typename std::remove_const<typename TSet::ElementType>::type;
			set.commit(PruningBoundary<BoundaryType>());
		}

		template<typename TElementTraits>
		struct OrderedSetTraits {
			using Type = OrderedSet<TElementTraits>;
			using DeltaType = OrderedSetDelta<TElementTraits>;
			using ElementType = typename TElementTraits::ElementType;
			using SetTraits = typename DeltaType::SetTraits;

			static auto Create() {
				return std::make_shared<OrderedSet<TElementTraits>>();
			}

			static void Commit(Type& set) {
				CommitWithoutPruning(set);
			}
		};

		template<typename TMutabilityTraits>
		using OrderedSetTypeTraits = OrderedSetTraits<TMutabilityTraits>;

		using MutableTraits = OrderedSetTypeTraits<MutableTypeTraits<test::MutableTestElement>>;
		using MutablePointerTraits = OrderedSetTypeTraits<MutableTypeTraits<std::shared_ptr<test::MutableTestElement>>>;
		using ImmutableTraits = OrderedSetTypeTraits<ImmutableTypeTraits<const test::ImmutableTestElement>>;
		using ImmutablePointerTraits = OrderedSetTypeTraits<ImmutableTypeTraits<std::shared_ptr<const test::ImmutableTestElement>>>;
	}

#define MAKE_ORDERED_SET_TEST(TEST_NAME, TRAITS_TYPE, TRAITS_NAME) \
	TEST(OrderedSetTests, TEST_NAME##_##TRAITS_NAME) { TRAITS_TEST_NAME(OrderedSetTests, TEST_NAME)<TRAITS_TYPE>(); }

#define REGISTER_DELTA_TESTS(TEST_NAME) \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::DeltaTraits<MutablePointerTraits>, DeltaMutablePointer); \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::DeltaTraits<ImmutablePointerTraits>, DeltaImmutablePointer); \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::DeltaTraits<MutableTraits>, DeltaMutable); \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::DeltaTraits<ImmutableTraits>, DeltaImmutable); \

#define REGISTER_NON_DELTA_TESTS(TEST_NAME) \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::BaseTraits<MutablePointerTraits>, BaseMutablePointer); \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::BaseTraits<ImmutablePointerTraits>, BaseImmutablePointer); \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::BaseTraits<MutableTraits>, BaseMutable); \
	MAKE_ORDERED_SET_TEST(TEST_NAME, test::BaseTraits<ImmutableTraits>, BaseImmutable); \

#define REGISTER_ALL_TESTS(TEST_NAME) \
	REGISTER_DELTA_TESTS(TEST_NAME) \
	REGISTER_NON_DELTA_TESTS(TEST_NAME) \

#define DEFINE_ORDERED_SET_TESTS(TEST_NAME, TYPES) \
	template<typename TTraits> void TRAITS_TEST_NAME(OrderedSetTests, TEST_NAME)(); \
	TYPES(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(OrderedSetTests, TEST_NAME)()

#define DEFINE_NON_DELTA_TESTS(TEST_NAME) DEFINE_ORDERED_SET_TESTS(TEST_NAME, REGISTER_NON_DELTA_TESTS)
#define DEFINE_DELTA_AND_NON_DELTA_TESTS(TEST_NAME) DEFINE_ORDERED_SET_TESTS(TEST_NAME, REGISTER_ALL_TESTS)

	// region ctor

	DEFINE_DELTA_AND_NON_DELTA_TESTS(CanCreateOrderedSet) {
		// Act:
		auto pSet = TTraits::Create();

		// Assert:
		TTraits::AssertContents(*pSet, typename TTraits::ElementVector());
	}

	// endregion

	// region rebase

	DEFINE_NON_DELTA_TESTS(RebaseCreatesOrderedSetAroundSuppliedElements) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);

		// Act:
		auto pDelta = pSet->rebase();

		// Assert:
		TTraits::AssertContents(*pDelta, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(RebaseAllowsOnlyOneAttachedDeltaAtATime) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);
		{
			auto pDelta = pSet->rebase();

			// Act + Assert:
			EXPECT_THROW(pSet->rebase(), catapult_runtime_error);
		}

		// Act: delta went out of scope, another delta is allowed
		auto pDelta = pSet->rebase();
		TTraits::AssertContents(*pDelta, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(RebaseDetachedCreatesOrderedSetAroundSuppliedElements) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);

		// Act:
		auto pDetachedDelta = pSet->rebaseDetached();

		// Assert:
		TTraits::AssertContents(*pDetachedDelta, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(RebaseDetachedAllowsManyDeltas) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto expectedElements = TTraits::CreateElements(3);
		std::vector<decltype(pSet->rebaseDetached())> deltas;

		// Act:
		for (auto i = 0u; i < 10; ++i)
			deltas.push_back(pSet->rebaseDetached());

		// Assert:
		for (const auto& pDelta : deltas)
			TTraits::AssertContents(*pDelta, expectedElements);
	}

	// endregion

	// region commit

	DEFINE_NON_DELTA_TESTS(CommitWithUnsetPruningBoundaryCommitsToOriginalElementsWithoutPruning) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(3);
		auto pDelta = pSet->rebase();
		pDelta->emplace("MyTestElement", 123u);
		pDelta->remove(TTraits::CreateElement("TestElement", 0));
		pDelta->remove(TTraits::CreateElement("TestElement", 2));
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 1),
			TTraits::CreateElement("MyTestElement", 123),
		};

		// Act:
		CommitWithoutPruning(*pSet);

		// Assert:
		TTraits::AssertContents(*pSet, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(CommitPrunesElementsPreviousToPruningBoundary) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4),
		};
		auto pruningBoundary = TTraits::CreateElement("TestElement", 3);

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(CommitIsNullOperationIfPruningBoundaryIsEqualToFirstSetElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();
		pDelta->remove(TTraits::CreateElement("TestElement", 0));
		pDelta->remove(TTraits::CreateElement("TestElement", 1));
		CommitWithoutPruning(*pSet);
		auto pruningBoundary = TTraits::CreateElement("TestElement", 2);
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 2),
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4),
		};

		// Sanity check:
		auto firstElement = *pSet->begin();
		EXPECT_EQ(*TTraits::ToPointer(firstElement), *TTraits::ToPointer(pruningBoundary));

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(CommitIsNullOperationIfPruningBoundaryIsSmallerThanFirstSetElement) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();
		pDelta->remove(TTraits::CreateElement("TestElement", 0));
		pDelta->remove(TTraits::CreateElement("TestElement", 1));
		CommitWithoutPruning(*pSet);
		auto pruningBoundary = TTraits::CreateElement("TestElement", 1);
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 2),
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4),
		};

		// Sanity check:
		auto firstElement = *pSet->begin();
		EXPECT_TRUE(*TTraits::ToPointer(pruningBoundary) < *TTraits::ToPointer(firstElement));

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(CommitPruningBoundaryDoesNotNeedToBeInSet) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();
		auto pruningBoundary = TTraits::CreateElement("TestElement", 2);
		pDelta->remove(pruningBoundary);
		CommitWithoutPruning(*pSet);
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4),
		};

		// Sanity check:
		EXPECT_FALSE(!!pSet->find(pruningBoundary));

		// Act:
		pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(CommitIsIdempotent) {
		// Arrange:
		auto pSet = TTraits::CreateWithElements(5);
		auto pDelta = pSet->rebase();
		auto pruningBoundary = TTraits::CreateElement("TestElement", 3);
		auto expectedElements = typename TTraits::ElementVector{
			TTraits::CreateElement("TestElement", 3),
			TTraits::CreateElement("TestElement", 4),
		};

		// Act:
		for (auto i = 0u; i < 10u; ++i)
			pSet->commit(pruningBoundary);

		// Assert:
		TTraits::AssertContents(*pSet, expectedElements);
	}

	DEFINE_NON_DELTA_TESTS(CommitThrowsIfOnlyDetachedDeltasAreOutstanding) {
		// Arrange:
		auto pSet = TTraits::Create();
		auto pDetachedDelta = pSet->rebaseDetached();

		// Act + Assert:
		EXPECT_THROW(CommitWithoutPruning(*pSet), catapult_runtime_error);
	}

	// endregion
}}
