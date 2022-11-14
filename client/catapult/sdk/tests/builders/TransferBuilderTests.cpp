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

#include "src/builders/TransferBuilder.h"
#include "src/extensions/ConversionExtensions.h"
#include "src/extensions/IdGenerator.h"
#include "catapult/crypto/Hashes.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"
#include <map>

namespace catapult { namespace builders {

#define TEST_CLASS TransferBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::TransferTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedTransferTransaction>;

		using TransactionType = model::TransferTransaction;
		using TransactionPtr = std::unique_ptr<TransactionType>;

		struct TransactionProperties {
		public:
			TransactionProperties() : Recipient(extensions::CopyToUnresolvedAddress(test::GenerateRandomByteArray<Address>()))
			{}

		public:
			UnresolvedAddress Recipient;
			std::vector<uint8_t> Message;
			std::vector<model::UnresolvedMosaic> Mosaics;
		};

		template<typename TTransaction>
		void AssertTransactionProperties(const TransactionProperties& expectedProperties, const TTransaction& transaction) {
			EXPECT_EQ(expectedProperties.Recipient, transaction.RecipientAddress);

			ASSERT_EQ(expectedProperties.Message.size(), transaction.MessageSize);
			EXPECT_EQ_MEMORY(expectedProperties.Message.data(), transaction.MessagePtr(), expectedProperties.Message.size());

			// - note mosaic comparison preserves order
			ASSERT_EQ(expectedProperties.Mosaics.size(), transaction.MosaicsCount);
			const auto* pActualMosaic = transaction.MosaicsPtr();
			for (auto i = 0u; i < transaction.MosaicsCount; ++i, ++pActualMosaic) {
				EXPECT_EQ(expectedProperties.Mosaics[i].MosaicId, pActualMosaic->MosaicId) << "mosaic at " << i;
				EXPECT_EQ(expectedProperties.Mosaics[i].Amount, pActualMosaic->Amount) << "mosaic at " << i;
			}
		}

		template<typename TTraits>
		void AssertCanBuildTransfer(
				size_t additionalSize,
				const TransactionProperties& expectedProperties,
				const consumer<TransferBuilder&>& buildTransaction) {
			// Arrange:
			auto networkIdentifier = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomByteArray<Key>();

			// Act:
			TransferBuilder builder(networkIdentifier, signer);
			builder.setRecipientAddress(expectedProperties.Recipient);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckBuilderSize(additionalSize, builder);
			TTraits::CheckFields(additionalSize, *pTransaction);
			EXPECT_EQ(signer, pTransaction->SignerPublicKey);
			EXPECT_EQ(1u, pTransaction->Version);
			EXPECT_EQ(static_cast<model::NetworkIdentifier>(0x62), pTransaction->Network);
			EXPECT_EQ(model::Entity_Type_Transfer, pTransaction->Type);

			AssertTransactionProperties(expectedProperties, *pTransaction);
		}

		void RunBuilderTest(const consumer<TransferBuilder&>& buildTransaction) {
			// Arrange:
			TransferBuilder builder(static_cast<model::NetworkIdentifier>(0x62), test::GenerateRandomByteArray<Key>());
			builder.setRecipientAddress(extensions::CopyToUnresolvedAddress(test::GenerateRandomByteArray<Address>()));

			// Act:
			buildTransaction(builder);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region basic

	TRAITS_BASED_TEST(CanCreateTransferWithoutMessageOrMosaics) {
		AssertCanBuildTransfer<TTraits>(0, TransactionProperties(), [](const auto&) {});
	}

	// endregion

	// region message

	namespace {
		struct BinaryMessageTraits {
			static constexpr auto GenerateRandomMessage = test::GenerateRandomVector;

			static void SetMessage(TransferBuilder& builder, const std::vector<uint8_t>& message) {
				builder.setMessage(message);
			}
		};
	}

#define TRAITS_BASED_MESSAGE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Binary) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Binary) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_MESSAGE_TEST(CanCreateTransferWithMessage) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Message = BinaryMessageTraits::GenerateRandomMessage(212);

		// Act:
		auto additionalSize = expectedProperties.Message.size();
		AssertCanBuildTransfer<TTraits>(additionalSize, expectedProperties, [&message = expectedProperties.Message](auto& builder) {
			BinaryMessageTraits::SetMessage(builder, message);
		});
	}

	TEST(TEST_CLASS, CannotSetEmptyMessage) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			// Act + Assert:
			EXPECT_THROW(BinaryMessageTraits::SetMessage(builder, {}), catapult_invalid_argument);
		});
	}

	TEST(TEST_CLASS, CannotSetMultipleMessages) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			BinaryMessageTraits::SetMessage(builder, BinaryMessageTraits::GenerateRandomMessage(212));

			// Act + Assert:
			EXPECT_THROW(
					BinaryMessageTraits::SetMessage(builder, BinaryMessageTraits::GenerateRandomMessage(212)),
					catapult_runtime_error);
		});
	}

	// endregion

	// region mosaics

	namespace {
		struct MosaicIdTraits {
			static std::vector<model::UnresolvedMosaic> GenerateMosaics(size_t count) {
				std::vector<model::UnresolvedMosaic> mosaics;
				for (auto i = 0u; i < count; ++i) {
					auto mosaic = model::UnresolvedMosaic{
						test::GenerateRandomValue<UnresolvedMosaicId>(),
						test::GenerateRandomValue<Amount>()
					};
					mosaics.push_back(mosaic);
				}

				std::sort(mosaics.begin(), mosaics.end(), [](const auto& lhs, const auto& rhs) {
					return lhs.MosaicId < rhs.MosaicId;
				});

				return mosaics;
			}
		};

		void AddMosaic(TransferBuilder& builder, UnresolvedMosaicId mosaicId, Amount amount) {
			builder.addMosaic({ mosaicId, amount });
		}

		template<typename TTraits>
		void AssertCanCreateTransferWithMosaics(size_t numMosaics) {
			// Arrange:
			auto expectedProperties = TransactionProperties();
			expectedProperties.Mosaics = MosaicIdTraits::GenerateMosaics(numMosaics);

			// Act:
			auto additionalSize = expectedProperties.Mosaics.size() * sizeof(model::UnresolvedMosaic);
			AssertCanBuildTransfer<TTraits>(additionalSize, expectedProperties, [&mosaics = expectedProperties.Mosaics](auto& builder) {
				for (const auto& mosaic : mosaics)
					AddMosaic(builder, mosaic.MosaicId, mosaic.Amount);
			});
		}
	}

#define TRAITS_BASED_MOSAICS_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Id) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Id) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_MOSAICS_TEST(CanCreateTransferWithSingleMosaic) {
		AssertCanCreateTransferWithMosaics<TTraits>(1);
	}

	TRAITS_BASED_MOSAICS_TEST(CanCreateTransferWithMultipleMosaics) {
		AssertCanCreateTransferWithMosaics<TTraits>(3);
	}

	TRAITS_BASED_TEST(CannotAddSameMosaicMultipleTimes) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			auto mosaicPair = *MosaicIdTraits::GenerateMosaics(1).cbegin();

			// Act:
			AddMosaic(builder, mosaicPair.MosaicId, mosaicPair.Amount);

			// Act + Assert:
			EXPECT_THROW(AddMosaic(builder, mosaicPair.MosaicId, mosaicPair.Amount), catapult_runtime_error);
		});
	}

	TRAITS_BASED_TEST(MultipleMosaicsAreSortedByMosaicId) {
		// Arrange:
		auto expectedProperties = TransactionProperties();
		expectedProperties.Mosaics = {
			{ UnresolvedMosaicId(12), Amount(4'321) },
			{ UnresolvedMosaicId(23), Amount(3'321) },
			{ UnresolvedMosaicId(75), Amount(1'321) },
			{ UnresolvedMosaicId(99), Amount(7'321) }
		};

		auto additionalSize = 4 * sizeof(model::UnresolvedMosaic);
		AssertCanBuildTransfer<TTraits>(additionalSize, expectedProperties, [](auto& builder) {
			// Act: add mosaics out of order
			AddMosaic(builder, UnresolvedMosaicId(12), Amount(4'321));
			AddMosaic(builder, UnresolvedMosaicId(99), Amount(7'321));
			AddMosaic(builder, UnresolvedMosaicId(75), Amount(1'321));
			AddMosaic(builder, UnresolvedMosaicId(23), Amount(3'321));
		});
	}

	// endregion

	// region message and mosaics

	namespace {
		static void SetMessage(TransferBuilder& builder, const std::string& message) {
			builder.setMessage({ reinterpret_cast<const uint8_t*>(message.data()), message.size() });
		}
	}

	TRAITS_BASED_TEST(CanCreateTransferWithMessageAndMosaics) {
		// Arrange:
		auto message = std::string("this is a great transfer!");
		auto expectedProperties = TransactionProperties();
		expectedProperties.Message.resize(message.size());
		std::memcpy(&expectedProperties.Message[0], message.data(), message.size());
		expectedProperties.Mosaics = {
			{ UnresolvedMosaicId(0), Amount(4'321) },
			{ UnresolvedMosaicId(1234), Amount(1'000'000) }
		};

		// Act:
		auto additionalSize = message.size() + 2 * sizeof(model::UnresolvedMosaic);
		AssertCanBuildTransfer<TTraits>(additionalSize, expectedProperties, [&message](auto& builder) {
			AddMosaic(builder, UnresolvedMosaicId(0), Amount(4'321));
			SetMessage(builder, message);
			AddMosaic(builder, UnresolvedMosaicId(1234), Amount(1'000'000));
		});
	}

	// endregion
}}
