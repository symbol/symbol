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

#include "src/state/MetadataKey.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/MetadataTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MetadataKeyTests

	namespace {
#pragma pack(push, 1)

		struct PackedUniqueKey {
			Address SourceAddress;
			Address TargetAddress;
			uint64_t ScopedMetadataKey;
			uint64_t TargetId;
			model::MetadataType MetadataType;
		};

#pragma pack(pop)
	}

	// region MetadataKey

	namespace {
		Hash256 CalculateExpectedUniqueKey(
				const model::PartialMetadataKey& partialKey,
				model::MetadataType metadataType,
				uint64_t targetId) {
			auto packedUniqueKey = PackedUniqueKey{
				partialKey.SourceAddress,
				partialKey.TargetAddress,
				partialKey.ScopedMetadataKey,
				targetId,
				metadataType
			};

			Hash256 uniqueKey;
			crypto::Sha3_256({ reinterpret_cast<const uint8_t*>(&packedUniqueKey), sizeof(PackedUniqueKey) }, uniqueKey);
			return uniqueKey;
		}
	}

	TEST(TEST_CLASS, CanCreateMetadataKey_Account) {
		// Arrange:
		auto partialKey = test::GenerateRandomPartialMetadataKey();

		// Act:
		auto key = MetadataKey(partialKey);

		// Assert: target dependent properties
		EXPECT_EQ(model::MetadataType::Account, key.metadataType());
		EXPECT_THROW(key.mosaicTarget(), catapult_invalid_argument);
		EXPECT_THROW(key.namespaceTarget(), catapult_invalid_argument);

		// - shared properties
		EXPECT_EQ(partialKey.SourceAddress, key.sourceAddress());
		EXPECT_EQ(partialKey.TargetAddress, key.targetAddress());
		EXPECT_EQ(partialKey.ScopedMetadataKey, key.scopedMetadataKey());
		EXPECT_EQ(0u, key.targetId());

		// - unique key
		auto expectedUniqueKey = CalculateExpectedUniqueKey(partialKey, model::MetadataType::Account, 0);
		EXPECT_EQ(expectedUniqueKey, key.uniqueKey());
	}

	namespace {
		template<typename TTargetIdentifier, typename TAction>
		void RunMetadataKeyTestWithTarget(model::MetadataType expectedMetadataType, TTargetIdentifier targetId, TAction action) {
			// Arrange:
			auto partialKey = test::GenerateRandomPartialMetadataKey();

			// Act:
			auto key = MetadataKey(partialKey, targetId);

			// Assert: target dependent properties
			EXPECT_EQ(expectedMetadataType, key.metadataType());
			action(key);

			// - shared properties
			EXPECT_EQ(partialKey.SourceAddress, key.sourceAddress());
			EXPECT_EQ(partialKey.TargetAddress, key.targetAddress());
			EXPECT_EQ(partialKey.ScopedMetadataKey, key.scopedMetadataKey());
			EXPECT_EQ(targetId, TTargetIdentifier(key.targetId()));

			// - unique key
			auto expectedUniqueKey = CalculateExpectedUniqueKey(partialKey, expectedMetadataType, targetId.unwrap());
			EXPECT_EQ(expectedUniqueKey, key.uniqueKey());
		}
	}

	TEST(TEST_CLASS, CanCreateMetadataKey_Mosaic) {
		// Act:
		auto mosaicId = test::GenerateRandomValue<MosaicId>();
		RunMetadataKeyTestWithTarget(model::MetadataType::Mosaic, mosaicId, [mosaicId](const auto& key) {
			// Assert:
			EXPECT_EQ(mosaicId, key.mosaicTarget());
			EXPECT_THROW(key.namespaceTarget(), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, CanCreateMetadataKey_Namespace) {
		// Act:
		auto namespaceId = test::GenerateRandomValue<NamespaceId>();
		RunMetadataKeyTestWithTarget(model::MetadataType::Namespace, namespaceId, [namespaceId](const auto& key) {
			// Assert:
			EXPECT_THROW(key.mosaicTarget(), catapult_invalid_argument);
			EXPECT_EQ(namespaceId, key.namespaceTarget());
		});
	}

	// endregion

	// region CreateMetadataKey

	namespace {
		template<typename TTargetIdChecker>
		void AssertCanCreateMetadataKey(model::MetadataType metadataType, TTargetIdChecker targetIdChecker) {
			// Arrange:
			auto partialKey = test::GenerateRandomPartialMetadataKey();
			auto target = model::MetadataTarget{ metadataType, test::Random() };

			// Act:
			auto key = CreateMetadataKey(partialKey, target);

			// Assert:
			EXPECT_EQ(partialKey.SourceAddress, key.sourceAddress());
			EXPECT_EQ(partialKey.TargetAddress, key.targetAddress());
			EXPECT_EQ(partialKey.ScopedMetadataKey, key.scopedMetadataKey());

			EXPECT_EQ(metadataType, key.metadataType());
			targetIdChecker(target.Id, key.targetId());
		}
	}

	TEST(TEST_CLASS, CreateMetadataKey_CanCreateFromAccountTarget) {
		AssertCanCreateMetadataKey(model::MetadataType::Account, [](auto, auto keyTargetId) {
			EXPECT_EQ(0u, keyTargetId);
		});
	}

	TEST(TEST_CLASS, CreateMetadataKey_CanCreateFromMosaicTarget) {
		AssertCanCreateMetadataKey(model::MetadataType::Mosaic, [](auto seedTargetId, auto keyTargetId) {
			EXPECT_EQ(seedTargetId, keyTargetId);
		});
	}

	TEST(TEST_CLASS, CreateMetadataKey_CanCreateFromNamespaceTarget) {
		AssertCanCreateMetadataKey(model::MetadataType::Namespace, [](auto seedTargetId, auto keyTargetId) {
			EXPECT_EQ(seedTargetId, keyTargetId);
		});
	}

	TEST(TEST_CLASS, CreateMetadataKey_CannotCreateFromOtherTarget) {
		// Arrange:
		auto partialKey = test::GenerateRandomPartialMetadataKey();
		auto target = model::MetadataTarget{ static_cast<model::MetadataType>(0xFF), test::Random() };

		// Act + Assert:
		EXPECT_THROW(CreateMetadataKey(partialKey, target), catapult_invalid_argument);
	}

	// endregion

	// region ResolveMetadataKey

	namespace {
		model::UnresolvedPartialMetadataKey UnresolveXor(const model::PartialMetadataKey& partialKey) {
			return { partialKey.SourceAddress, test::UnresolveXor(partialKey.TargetAddress), partialKey.ScopedMetadataKey };
		}

		template<typename TTargetIdChecker>
		void AssertCanResolveMetadataKey(model::MetadataType metadataType, TTargetIdChecker targetIdChecker) {
			// Arrange:
			auto partialKey = test::GenerateRandomPartialMetadataKey();
			auto target = model::MetadataTarget{ metadataType, test::Random() };

			// Act:
			auto key = ResolveMetadataKey(UnresolveXor(partialKey), target, test::CreateResolverContextXor());

			// Assert:
			EXPECT_EQ(partialKey.SourceAddress, key.sourceAddress());
			EXPECT_EQ(partialKey.TargetAddress, key.targetAddress());
			EXPECT_EQ(partialKey.ScopedMetadataKey, key.scopedMetadataKey());

			EXPECT_EQ(metadataType, key.metadataType());
			targetIdChecker(target.Id, key.targetId());
		}
	}

	TEST(TEST_CLASS, ResolveMetadataKey_CanResolveFromAccountTarget) {
		AssertCanResolveMetadataKey(model::MetadataType::Account, [](auto, auto keyTargetId) {
			EXPECT_EQ(0u, keyTargetId);
		});
	}

	TEST(TEST_CLASS, ResolveMetadataKey_CanResolveFromMosaicTarget) {
		AssertCanResolveMetadataKey(model::MetadataType::Mosaic, [](auto seedTargetId, auto keyTargetId) {
			EXPECT_EQ(UnresolvedMosaicId(seedTargetId), test::UnresolveXor(MosaicId(keyTargetId)));
		});
	}

	TEST(TEST_CLASS, ResolveMetadataKey_CanResolveFromNamespaceTarget) {
		AssertCanResolveMetadataKey(model::MetadataType::Namespace, [](auto seedTargetId, auto keyTargetId) {
			EXPECT_EQ(seedTargetId, keyTargetId);
		});
	}

	TEST(TEST_CLASS, ResolveMetadataKey_CannotResolveFromOtherTarget) {
		// Arrange:
		auto partialKey = test::GenerateRandomPartialMetadataKey();
		auto target = model::MetadataTarget{ static_cast<model::MetadataType>(0xFF), test::Random() };

		// Act + Assert:
		EXPECT_THROW(ResolveMetadataKey(UnresolveXor(partialKey), target, test::CreateResolverContextXor()), catapult_invalid_argument);
	}

	// endregion
}}
