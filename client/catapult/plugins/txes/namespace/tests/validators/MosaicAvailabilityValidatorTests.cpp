#include "src/validators/Validators.h"
#include "src/cache/MosaicCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicAvailabilityValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicAvailability,)

	namespace {
		model::MosaicDefinitionNotification CreateNotification(
				const Key& signer,
				NamespaceId parentId,
				MosaicId id,
				ArtifactDuration duration) {
			return model::MosaicDefinitionNotification(
					signer,
					parentId,
					id,
					model::MosaicProperties::FromValues({ { 0, 0, duration.unwrap() } }));
		}

		model::MosaicDefinitionNotification CreateNotification(const Key& signer, NamespaceId parentId, MosaicId id) {
			return CreateNotification(signer, parentId, id, Eternal_Artifact_Duration);
		}

		model::MosaicDefinitionNotification CreateNotification(const Key& signer, MosaicId id) {
			return CreateNotification(signer, NamespaceId(), id, Eternal_Artifact_Duration);
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				Height height,
				const model::MosaicDefinitionNotification& notification) {
			// Arrange:
			auto pValidator = CreateMosaicAvailabilityValidator();

			// - create the validator context
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height << ", id " << notification.MosaicId;
		}

		void AddMosaic(cache::CatapultCache& cache, MosaicId id, Amount mosaicSupply, const Key& owner, Amount ownerSupply) {
			auto delta = cache.createDelta();
			test::AddMosaic(delta, id, Height(50), ArtifactDuration(100), mosaicSupply);
			test::AddMosaicOwner(delta, id, owner, ownerSupply);
			cache.commit(Height());
		}

		void AddEternalMosaic(cache::CatapultCache& cache, NamespaceId parentId, MosaicId id, const Key& owner) {
			auto delta = cache.createDelta();
			test::AddEternalMosaic(delta, parentId, id, Height(50), owner);
			test::AddMosaicOwner(delta, id, owner, Amount());
			cache.commit(Height());
		}
	}

	// region active mosaic check

	TEST(TEST_CLASS, CanAddUnknownMosaic) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, MosaicId(123));

		// - seed the cache with an unrelated mosaic
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(100), Amount(500), signer, Amount(400));

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
	}

	TEST(TEST_CLASS, CanReplaceInactiveMosaic) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, MosaicId(123));

		// - seed the cache with an expired mosaic with the same id
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(500), signer, Amount(400));

		// Assert: notice that the mosaic expires at height 150
		AssertValidationResult(ValidationResult::Success, cache, Height(200), notification);
	}

	// endregion

	// region parent check

	TEST(TEST_CLASS, CanReplaceKnownMosaicHavingSameParentNamespace) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, NamespaceId(234), MosaicId(123), ArtifactDuration(100));

		// - seed the cache with a mosaic with the same id and same parent id
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddEternalMosaic(cache, NamespaceId(234), MosaicId(123), signer);

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, Height(200), notification);
	}

	TEST(TEST_CLASS, CannotReplaceKnownMosaicHavingDifferentParentNamespace) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, NamespaceId(345), MosaicId(123), ArtifactDuration(100));

		// - seed the cache with a mosaic with the same id and different parent id
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddEternalMosaic(cache, NamespaceId(234), MosaicId(123), signer);

		// Assert:
		AssertValidationResult(Failure_Mosaic_Parent_Id_Conflict, cache, Height(200), notification);
	}

	// endregion

	// region properties check

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenDefinitionIsUnchanged) {
		// Arrange: create a transaction with matching properties
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, NamespaceId(11), MosaicId(123), ArtifactDuration(100));

		// - seed the cache with an active mosaic with the same id (notice that added mosaic has duration of 100)
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(500), signer, Amount(500));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Modification_No_Changes, cache, Height(100), notification);
	}

	// endregion

	// region full supply check

	TEST(TEST_CLASS, CanReplaceActiveMosaicWhenOwnerHasFullSupply) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, NamespaceId(11), MosaicId(123));

		// - seed the cache with an active mosaic with the same id
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(500), signer, Amount(500));

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, Height(100), notification);
	}

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenOwnerHasLessThanFullSupply) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, NamespaceId(11), MosaicId(123));

		// - seed the cache with an active mosaic with the same id
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddMosaic(cache, MosaicId(123), Amount(500), signer, Amount(400));

		// Assert:
		AssertValidationResult(Failure_Mosaic_Modification_Disallowed, cache, Height(100), notification);
	}

	TEST(TEST_CLASS, CannotReplaceActiveMosaicWhenOwnerIsUnknown) {
		// Arrange:
		auto signer = test::GenerateRandomData<Key_Size>();
		auto notification = CreateNotification(signer, NamespaceId(11), MosaicId(123));

		// - seed the cache with an active mosaic with the same id but with an unknown owner
		auto cache = test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		auto delta = cache.createDelta();
		test::AddMosaic(delta, MosaicId(123), Height(50), ArtifactDuration(100), Amount(500));
		cache.commit(Height());

		// Assert:
		AssertValidationResult(Failure_Mosaic_Modification_Disallowed, cache, Height(100), notification);
	}

	// endregion
}}
