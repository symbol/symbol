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

#include "src/builders/TransferBuilder.h"
#include "src/extensions/IdGenerator.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/constants.h"
#include "sdk/tests/builders/test/BuilderTestUtils.h"
#include <map>

namespace catapult { namespace builders {

#define TEST_CLASS TransferBuilderTests

	namespace {
		using RegularTraits = test::RegularTransactionTraits<model::TransferTransaction>;
		using EmbeddedTraits = test::EmbeddedTransactionTraits<model::EmbeddedTransferTransaction>;

		void RunBuilderTest(const consumer<TransferBuilder&>& buildTransfer) {
			// Arrange:
			TransferBuilder builder(
					static_cast<model::NetworkIdentifier>(0x62),
					test::GenerateRandomData<Key_Size>(),
					test::GenerateRandomData<Address_Decoded_Size>());

			// Act:
			buildTransfer(builder);
		}

		template<typename TTraits, typename TValidationFunction>
		void AssertCanBuildTransfer(
				size_t additionalSize,
				const consumer<TransferBuilder&>& buildTransaction,
				const TValidationFunction& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();
			auto recipient = test::GenerateRandomData<Address_Decoded_Size>();

			// Act:
			TransferBuilder builder(networkId, signer, recipient);
			buildTransaction(builder);
			auto pTransaction = TTraits::InvokeBuilder(builder);

			// Assert:
			TTraits::CheckFields(additionalSize, *pTransaction);

			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6203, pTransaction->Version);
			EXPECT_EQ(model::Entity_Type_Transfer, pTransaction->Type);

			EXPECT_EQ(recipient, pTransaction->Recipient);
			validateTransaction(*pTransaction);
		}
	}

#define TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// region basic

	TRAITS_BASED_TEST(CanCreateTransferWithoutMessageOrMosaics) {
		// Act:
		AssertCanBuildTransfer<TTraits>(
				0,
				[](const auto&) {},
				[](const auto& transfer) {
					// Assert: neither a message nor mosaics are present
					EXPECT_EQ(0u, transfer.MessageSize);
					EXPECT_EQ(0u, transfer.MosaicsCount);
				});
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

		struct StringMessageTraits {
			static constexpr auto GenerateRandomMessage = test::GenerateRandomString;

			static void SetMessage(TransferBuilder& builder, const std::string& message) {
				builder.setStringMessage(message);
			}
		};
	}

#define TRAITS_BASED_MESSAGE_TEST(TEST_NAME) \
	template<typename TTraits, typename TMessageTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Binary) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits, BinaryMessageTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Regular_String) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits, StringMessageTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Binary) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits, BinaryMessageTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_String) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits, StringMessageTraits>(); } \
	template<typename TTraits, typename TMessageTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_MESSAGE_TEST(CanCreateTransferWithMessage) {
		// Arrange:
		auto message = TMessageTraits::GenerateRandomMessage(212);

		// Act:
		AssertCanBuildTransfer<TTraits>(
				message.size(),
				[&message](auto& builder) {
					TMessageTraits::SetMessage(builder, message);
				},
				[&message](const auto& transfer) {
					// Assert: a message is present
					auto expectedMessageSize = message.size();
					auto actualMessageSize = transfer.MessageSize;
					EXPECT_EQ(expectedMessageSize, actualMessageSize);
					EXPECT_TRUE(0 == std::memcmp(message.data(), transfer.MessagePtr(), message.size()));

					// - no mosaics are present
					EXPECT_EQ(0u, transfer.MosaicsCount);
				});
	}

#define MESSAGE_ACCESSOR_TEST(TEST_NAME) \
	template<typename TMessageTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Binary) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BinaryMessageTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_String) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StringMessageTraits>(); } \
	template<typename TMessageTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	MESSAGE_ACCESSOR_TEST(CannotSetEmptyMessage) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			// Act + Assert:
			EXPECT_THROW(TMessageTraits::SetMessage(builder, {}), catapult_invalid_argument);
		});
	}

	MESSAGE_ACCESSOR_TEST(CannotSetMultipleMessages) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			TMessageTraits::SetMessage(builder, TMessageTraits::GenerateRandomMessage(212));

			// Act + Assert:
			EXPECT_THROW(TMessageTraits::SetMessage(builder, TMessageTraits::GenerateRandomMessage(212)), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, CannotSetMultipleMessages_DifferentTypes) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			auto message1 = test::GenerateRandomVector(123);
			auto message2 = test::GenerateRandomString(212);

			builder.setMessage(message1);

			// Act + Assert:
			EXPECT_THROW(builder.setStringMessage(message2), catapult_runtime_error);
		});
	}

	// endregion

	// region mosaics

	namespace {
		template<typename TTransaction>
		std::map<MosaicId, Amount> ExtractMosaicsMap(const TTransaction& transfer) {
			std::map<MosaicId, Amount> mosaics;
			auto pMosaic = transfer.MosaicsPtr();
			for (auto i = 0u; i < transfer.MosaicsCount; ++i) {
				mosaics.emplace(pMosaic->MosaicId, pMosaic->Amount);
				++pMosaic;
			}

			return mosaics;
		}

		struct MosaicIdTraits {
			static std::map<MosaicId, Amount> GenerateMosaics(size_t count) {
				std::map<MosaicId, Amount> mosaics;
				for (auto i = 0u; i < count; ++i)
					mosaics.emplace(test::GenerateRandomValue<MosaicId>(), test::GenerateRandomValue<Amount>());

				return mosaics;
			}

			static const std::map<MosaicId, Amount>& ToMosaicsMap(const std::map<MosaicId, Amount>& mosaics) {
				return mosaics;
			}
		};

		struct MosaicNameTraits {
			static std::map<std::string, Amount> GenerateMosaics(size_t count) {
				std::map<std::string, Amount> mosaics;
				for (auto i = 0u; i < count; ++i) {
					auto mosaicName = test::GenerateRandomHexString(4) + ":" + test::GenerateRandomHexString(i + 4);
					mosaics.emplace(mosaicName, test::GenerateRandomValue<Amount>());
				}

				return mosaics;
			}

			static std::map<MosaicId, Amount> ToMosaicsMap(const std::map<std::string, Amount>& namedMosaics) {
				std::map<MosaicId, Amount> mosaics;
				for (const auto& namedMosaic : namedMosaics)
					mosaics.emplace(extensions::GenerateMosaicId(namedMosaic.first), namedMosaic.second);

				return mosaics;
			}
		};

		template<typename TTraits, typename TMosaicTraits>
		void AssertCanCreateTransferWithMosaics(size_t numMosaics) {
			// Arrange:
			auto mosaics = TMosaicTraits::GenerateMosaics(numMosaics);

			// Act:
			AssertCanBuildTransfer<TTraits>(
					mosaics.size() * sizeof(model::Mosaic),
					[&mosaics](auto& builder) {
						for (const auto& mosaic : mosaics)
							builder.addMosaic(mosaic.first, mosaic.second);
					},
					[numMosaics, &mosaics](const auto& transfer) {
						// Assert: no message is present
						EXPECT_EQ(0u, transfer.MessageSize);

						// - mosaics are present
						EXPECT_EQ(numMosaics, transfer.MosaicsCount);
						const auto& expectedMosaics = TMosaicTraits::ToMosaicsMap(mosaics);
						const auto& actualMosaics = ExtractMosaicsMap(transfer);
						EXPECT_EQ(expectedMosaics, actualMosaics);
					});
		}
	}

#define TRAITS_BASED_MOSAICS_TEST(TEST_NAME) \
	template<typename TTraits, typename TMosaicTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Id) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits, MosaicIdTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Regular_Name) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits, MosaicNameTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Id) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits, MosaicIdTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_Name) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits, MosaicNameTraits>(); } \
	template<typename TTraits, typename TMosaicTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAITS_BASED_MOSAICS_TEST(CanCreateTransferWithSingleMosaic) {
		// Assert:
		AssertCanCreateTransferWithMosaics<TTraits, TMosaicTraits>(1);
	}

	TRAITS_BASED_MOSAICS_TEST(CanCreateTransferWithMultipleMosaics) {
		// Assert:
		AssertCanCreateTransferWithMosaics<TTraits, TMosaicTraits>(3);
	}

#define MOSAICS_ACCESSOR_TEST(TEST_NAME) \
	template<typename TMosaicTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Id) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicIdTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Name) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicNameTraits>(); } \
	template<typename TMosaicTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	MOSAICS_ACCESSOR_TEST(CannotAddSameMosaicMultipleTimes) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			auto mosaic = *TMosaicTraits::GenerateMosaics(1).cbegin();

			// Act:
			builder.addMosaic(mosaic.first, mosaic.second);

			// Act + Assert:
			EXPECT_THROW(builder.addMosaic(mosaic.first, mosaic.second), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, CannotAddSameMosaicMultipleTimes_DifferentIdTypes) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			// Act:
			builder.addMosaic(Xem_Id, Amount(123));

			// Act + Assert:
			EXPECT_THROW(builder.addMosaic("nem:xem", Amount(234)), catapult_runtime_error);
		});
	}

	TRAITS_BASED_TEST(MultipleMosaicsAreSortedByMosaicId) {
		// Arrange:
		AssertCanBuildTransfer<TTraits>(
				4 * sizeof(model::Mosaic),
				[](auto& builder) {
					// Act: add mosaics out of order
					builder.addMosaic(MosaicId(12), Amount(4'321));
					builder.addMosaic(MosaicId(99), Amount(7'321));
					builder.addMosaic(MosaicId(75), Amount(1'321));
					builder.addMosaic(MosaicId(23), Amount(3'321));
				},
				[](const auto& transfer) {
					// Assert: four mosaics are present and they are sorted by id
					auto pMosaics = transfer.MosaicsPtr();
					ASSERT_EQ(4u, transfer.MosaicsCount);
					EXPECT_EQ(MosaicId(12), pMosaics[0].MosaicId);
					EXPECT_EQ(MosaicId(23), pMosaics[1].MosaicId);
					EXPECT_EQ(MosaicId(75), pMosaics[2].MosaicId);
					EXPECT_EQ(MosaicId(99), pMosaics[3].MosaicId);
				});
	}

	// endregion

	// region message and mosaics

	TRAITS_BASED_TEST(CanCreateTransferWithMessageAndMosaics) {
		// Arrange:
		auto message = std::string("this is a great transfer!");

		// Act:
		AssertCanBuildTransfer<TTraits>(
				message.size() + 2 * sizeof(model::Mosaic),
				[&message](auto& builder) {
					builder.addMosaic(MosaicId(0), Amount(4'321));
					builder.setStringMessage(message);
					builder.addMosaic("nem:xem", Amount(1'000'000));
				},
				[&message](const auto& transfer) {
					// Assert: a message is present
					EXPECT_EQ(message.size(), transfer.MessageSize);
					EXPECT_TRUE(0 == std::memcmp(message.data(), transfer.MessagePtr(), message.size()));

					// - two mosaics are present
					auto expectedMosaicsMap = std::map<MosaicId, Amount>{{
						{ Xem_Id, Amount(1'000'000) },
						{ MosaicId(0), Amount(4'321) }
					}};
					EXPECT_EQ(2u, transfer.MosaicsCount);
					EXPECT_EQ(expectedMosaicsMap, ExtractMosaicsMap(transfer));
				});
	}

	// endregion
}}
