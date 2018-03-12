#include "src/validators/Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/constants.h"
#include "tests/test/MosaicCacheTestUtils.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/test/NamespaceCacheTestUtils.h"
#include "tests/test/NamespaceTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MosaicTransferValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MosaicTransfer,)

	namespace {
		auto CreateCache() {
			return test::MosaicCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		}

		state::RootNamespace CreateRootNamespace(NamespaceId id, const state::NamespaceLifetime& lifetime) {
			return state::RootNamespace(id, test::CreateRandomOwner(), lifetime);
		}

		model::MosaicProperties CreateMosaicProperties(model::MosaicFlags flags) {
			model::MosaicProperties::PropertyValuesContainer values{};
			values[utils::to_underlying_type(model::MosaicPropertyId::Flags)] = utils::to_underlying_type(flags);
			return model::MosaicProperties::FromValues(values);
		}

		state::MosaicDefinition CreateMosaicDefinition(Height height, const Key& owner, model::MosaicFlags flags) {
			return state::MosaicDefinition(height, owner, CreateMosaicProperties(flags));
		}

		state::MosaicEntry CreateMosaicEntry(NamespaceId namespaceId, MosaicId mosaicId, const Key& owner, model::MosaicFlags flags) {
			auto mosaicDefinition = CreateMosaicDefinition(Height(100), owner, flags);
			return state::MosaicEntry(namespaceId, mosaicId, mosaicDefinition);
		}

		void SeedCacheWithMosaic(cache::CatapultCache& cache, const state::MosaicEntry& mosaicEntry) {
			auto cacheDelta = cache.createDelta();
			auto& mosaicCacheDelta = cacheDelta.sub<cache::MosaicCache>();
			mosaicCacheDelta.insert(mosaicEntry);
			cache.commit(Height());
		}

		void SeedCacheWithNamespace(cache::CatapultCache& cache, const state::RootNamespace& rootNamespace) {
			auto cacheDelta = cache.createDelta();
			auto& namespaceCacheDelta = cacheDelta.sub<cache::NamespaceCache>();
			namespaceCacheDelta.insert(rootNamespace);
			cache.commit(Height());
		}

		void AssertValidationResult(
				ValidationResult expectedResult,
				const cache::CatapultCache& cache,
				const model::BalanceTransferNotification& notification) {
			// Arrange:
			auto pValidator = CreateMosaicTransferValidator();

			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto context = test::CreateValidatorContext(Height(100), readOnlyCache);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingXemTransfer) {
		// Arrange:
		auto notification = model::BalanceTransferNotification(Key(), Address(), Xem_Id, Amount(123));
		auto cache = CreateCache();

		// Assert:
		AssertValidationResult(ValidationResult::Success, cache, notification);
	}

	namespace {
		constexpr MosaicId Valid_Mosaic_Id(222);
		constexpr MosaicId Known_Mosaic_Id_Unknown_Ns(333);
		constexpr MosaicId Unknown_Mosaic_Id(444);

		auto CreateAndSeedCache(const Key& owner, model::MosaicFlags flags) {
			// Arrange:
			auto cache = CreateCache();
			auto validMosaicEntry = CreateMosaicEntry(NamespaceId(11111), Valid_Mosaic_Id, owner, flags);
			SeedCacheWithMosaic(cache, validMosaicEntry);
			SeedCacheWithNamespace(cache, CreateRootNamespace(NamespaceId(11111), test::CreateLifetime(50, 150)));

			SeedCacheWithMosaic(cache, CreateMosaicEntry(NamespaceId(55555), Known_Mosaic_Id_Unknown_Ns, owner, flags));

			auto cacheDelta = cache.createDelta();
			test::AddMosaicOwner(cacheDelta, Valid_Mosaic_Id, validMosaicEntry.definition().owner(), Amount());
			cache.commit(Height());

			return cache;
		}

		void AssertMosaicsTest(ValidationResult expectedResult, MosaicId mosaicId) {
			// Arrange:
			auto owner = test::GenerateRandomData<Key_Size>();
			auto notification = model::BalanceTransferNotification(owner, Address(), mosaicId, Amount(123));

			auto cache = CreateAndSeedCache(owner, model::MosaicFlags::Transferable);

			// Assert:
			AssertValidationResult(expectedResult, cache, notification);
		}
	}

	TEST(TEST_CLASS, FailureWhenValidatingUnknownMosaic) {
		// Assert:
		AssertMosaicsTest(Failure_Mosaic_Expired, Unknown_Mosaic_Id);
	}

	TEST(TEST_CLASS, FailureWhenValidatingMosaicFromUnknownNamespace) {
		// Assert:
		AssertMosaicsTest(Failure_Namespace_Expired, Known_Mosaic_Id_Unknown_Ns);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingKnownMosaicFromKnownNamespace) {
		// Assert:
		AssertMosaicsTest(ValidationResult::Success, Valid_Mosaic_Id);
	}

	namespace {
		enum : uint8_t {
			None = 0x00,
			Owner_Is_Sender = 0x01,
			Owner_Is_Recipient = 0x02
		};

		void AssertNonTransferableMosaicsTest(ValidationResult expectedResult, uint8_t notificationFlags) {
			// Arrange:
			auto owner = test::GenerateRandomData<Key_Size>();
			auto cache = CreateAndSeedCache(owner, model::MosaicFlags::None);

			// - notice that BalanceTransferNotification holds references to sender + recipient
			Key sender;
			Address recipient;
			auto notification = model::BalanceTransferNotification(sender, recipient, Valid_Mosaic_Id, Amount(123));

			if (notificationFlags & Owner_Is_Sender)
				sender = owner;

			if (notificationFlags & Owner_Is_Recipient)
				recipient = cache.createView().sub<cache::AccountStateCache>().get(owner).Address;

			// Assert:
			AssertValidationResult(expectedResult, cache, notification);
		}
	}

	TEST(TEST_CLASS, FailureWhenMosaicIsNonTransferableAndOwnerIsNotParticipant) {
		AssertNonTransferableMosaicsTest(Failure_Mosaic_Non_Transferable, None);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsNonTransferableAndOwnerIsSender) {
		AssertNonTransferableMosaicsTest(ValidationResult::Success, Owner_Is_Sender);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsNonTransferableAndOwnerIsRecipient) {
		AssertNonTransferableMosaicsTest(ValidationResult::Success, Owner_Is_Recipient);
	}

	TEST(TEST_CLASS, SuccessWhenMosaicIsNonTransferableAndOwnerSendsToSelf) {
		AssertNonTransferableMosaicsTest(ValidationResult::Success, Owner_Is_Recipient | Owner_Is_Sender);
	}
}}
