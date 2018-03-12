#include "src/handlers/NamespaceInfosSupplier.h"
#include "src/cache/NamespaceCache.h"
#include "src/model/NamespaceInfo.h"
#include "tests/test/ArtifactTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace handlers {

#define TEST_CLASS NamespaceInfosSupplierTests

	namespace {
		using NamespaceIds = std::vector<NamespaceId>;
		using NamespaceInfos = std::vector<std::shared_ptr<const model::NamespaceInfo>>;

		constexpr size_t Num_Namespace_Infos = 6;

		auto PrepareCache() {
			// tests use the following hierarchy
			// root 1, children { 2, 5, 6 }
			// root 3, children { 4 }
			auto pCache = std::make_unique<cache::NamespaceCache>();
			auto delta = pCache->createDelta();

			delta->insert(state::RootNamespace(NamespaceId(1), test::CreateRandomOwner(), test::CreateLifetime(1, 100)));
			delta->insert(state::Namespace(test::CreatePath({ 1, 2 })));
			delta->insert(state::Namespace(test::CreatePath({ 1, 2, 5 })));
			delta->insert(state::Namespace(test::CreatePath({ 1, 6 })));

			delta->insert(state::RootNamespace(NamespaceId(3), test::CreateRandomOwner(), test::CreateLifetime(1, 100)));
			delta->insert(state::Namespace(test::CreatePath({ 3, 4 })));

			pCache->commit();
			return pCache;
		}

		void AssertEqual(const NamespaceIds& expectedIds, const NamespaceInfos& actualInfos) {
			// Assert:
			ASSERT_EQ(expectedIds.size(), actualInfos.size());

			auto i = 0u;
			for (auto id : expectedIds) {
				auto pInfo = actualInfos[i];
				EXPECT_EQ(id, pInfo->Id) << "id at " << i;
				EXPECT_EQ(model::ArtifactInfoAttributes::Is_Known, pInfo->Attributes) << "attributes at " << i;
				++i;
			}
		}

		struct NamespaceInfosContext {
		public:
			NamespaceInfosContext()
					: pCache(PrepareCache())
					, Supplier(CreateNamespaceInfosSupplier(*pCache))
			{}

			std::unique_ptr<cache::NamespaceCache> pCache;
			NamespaceInfosSupplier Supplier;
		};

		void AssertCanSupplyNamespaceInfos(const std::vector<NamespaceId::ValueType>& ids) {
			// Arrange:
			NamespaceInfosContext context;
			auto expectedIds = test::ToArtifactIds<NamespaceId>(ids);
			auto idRange = model::EntityRange<NamespaceId>::CopyFixed(
					reinterpret_cast<const uint8_t*>(expectedIds.data()),
					expectedIds.size());

			// Act:
			auto namespaceInfos = context.Supplier(idRange);

			// Assert:
			AssertEqual(expectedIds, namespaceInfos);
		}
	}

	// region basic

	TEST(TEST_CLASS, CanSupplySingleNamespaceInfo) {
		// Assert:
		for (auto i = 1u; i <= Num_Namespace_Infos; ++i)
			AssertCanSupplyNamespaceInfos({ i });
	}

	TEST(TEST_CLASS, CanSupplyMultipleNamespaceInfos) {
		// Assert:
		AssertCanSupplyNamespaceInfos({ 1, 3, 5 });
		AssertCanSupplyNamespaceInfos({ 2, 4 });
	}

	TEST(TEST_CLASS, CanSupplyAllNamespaceInfos) {
		// Assert:
		auto ids = std::vector<NamespaceId::ValueType>(Num_Namespace_Infos);
		std::iota(ids.begin(), ids.end(), 1);
		AssertCanSupplyNamespaceInfos(ids);
	}

	// endregion

	// region different types of infos

	namespace {
		model::EntityRange<NamespaceId> CreateSingleIdRange(NamespaceId id) {
			return model::EntityRange<NamespaceId>::CopyFixed(reinterpret_cast<const uint8_t*>(&id), 1);
		}
	}

	TEST(TEST_CLASS, ReturnsInfoForUnkownNamespace) {
		// Arrange: request an unknown namespace to be supplied
		NamespaceInfosContext context;
		auto idRange = CreateSingleIdRange(NamespaceId(7));

		// Act:
		auto namespaceInfos = context.Supplier(idRange);

		// Assert:
		ASSERT_EQ(1u, namespaceInfos.size());

		const auto& pInfo = namespaceInfos[0];
		EXPECT_EQ(sizeof(model::NamespaceInfo), pInfo->Size);
		EXPECT_EQ(NamespaceId(7), pInfo->Id);
		EXPECT_EQ(model::ArtifactInfoAttributes::None, pInfo->Attributes);
		EXPECT_EQ(0u, pInfo->ChildCount);
	}

	TEST(TEST_CLASS, ReturnsInfoForChildNamespace) {
		// Arrange: request a child namespace to be supplied
		NamespaceInfosContext context;
		auto idRange = CreateSingleIdRange(NamespaceId(4));

		// Act:
		auto namespaceInfos = context.Supplier(idRange);

		// Assert:
		ASSERT_EQ(1u, namespaceInfos.size());

		const auto& pInfo = namespaceInfos[0];
		EXPECT_EQ(sizeof(model::NamespaceInfo), pInfo->Size);
		EXPECT_EQ(NamespaceId(4), pInfo->Id);
		EXPECT_EQ(model::ArtifactInfoAttributes::Is_Known, pInfo->Attributes);
		EXPECT_EQ(0u, pInfo->ChildCount);
	}

	TEST(TEST_CLASS, ReturnsInfoForRootNamespace) {
		// Arrange: request a root namespace to be supplied
		NamespaceInfosContext context;
		auto idRange = CreateSingleIdRange(NamespaceId(1));

		// Act:
		auto namespaceInfos = context.Supplier(idRange);

		// Assert:
		ASSERT_EQ(1u, namespaceInfos.size());

		const auto& pInfo = namespaceInfos[0];
		EXPECT_EQ(sizeof(model::NamespaceInfo) + 3 * sizeof(NamespaceId), pInfo->Size);
		EXPECT_EQ(NamespaceId(1), pInfo->Id);
		EXPECT_EQ(model::ArtifactInfoAttributes::Is_Known, pInfo->Attributes);
		ASSERT_EQ(3u, pInfo->ChildCount);

		// - check children (note that order is not important)
		const auto* pChildIds = pInfo->ChildrenPtr();
		std::set<NamespaceId> actualChildIds{ pChildIds[0], pChildIds[1], pChildIds[2] };
		EXPECT_EQ((std::set<NamespaceId>{ NamespaceId(2), NamespaceId(5), NamespaceId(6) }), actualChildIds);
	}

	// endregion
}}
