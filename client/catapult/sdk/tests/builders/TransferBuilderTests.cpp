#include "src/builders/TransferBuilder.h"
#include "src/extensions/IdGenerator.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/constants.h"
#include "tests/TestHarness.h"
#include <map>

#define TEST_CLASS TransferBuilderTests

namespace catapult { namespace builders {

	namespace {
		void RunBuilderTest(const std::function<void (TransferBuilder&)>& buildTransfer) {
			// Arrange:
			TransferBuilder builder(
					static_cast<model::NetworkIdentifier>(0x62),
					test::GenerateRandomData<Key_Size>(),
					test::GenerateRandomData<Address_Decoded_Size>());

			// Act:
			buildTransfer(builder);
		}

		void AssertCanBuildTransfer(
				size_t additionalSize,
				const std::function<void (TransferBuilder&)>& buildTransaction,
				const std::function<void (const model::TransferTransaction&)>& validateTransaction) {
			// Arrange:
			auto networkId = static_cast<model::NetworkIdentifier>(0x62);
			auto signer = test::GenerateRandomData<Key_Size>();
			auto recipient = test::GenerateRandomData<Address_Decoded_Size>();

			// Act:
			TransferBuilder builder(networkId, signer, recipient);
			buildTransaction(builder);
			auto pTransaction = builder.build();

			// Assert:
			ASSERT_EQ(sizeof(model::TransferTransaction) + additionalSize, pTransaction->Size);
			EXPECT_EQ(Signature{}, pTransaction->Signature);
			EXPECT_EQ(signer, pTransaction->Signer);
			EXPECT_EQ(0x6203, pTransaction->Version);
			EXPECT_EQ(model::EntityType::Transfer, pTransaction->Type);

			EXPECT_EQ(Amount(0), pTransaction->Fee);
			EXPECT_EQ(Timestamp(0), pTransaction->Deadline);

			EXPECT_EQ(recipient, pTransaction->Recipient);
			validateTransaction(*pTransaction);
		}
	}

	// region basic

	TEST(TEST_CLASS, CanCreateTransferWithoutMessageOrMosaics) {
		// Act:
		AssertCanBuildTransfer(
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

#define MESSAGE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Binary) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BinaryMessageTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_String) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<StringMessageTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	MESSAGE_TEST(CanCreateTransferWithMessage) {
		// Arrange:
		auto message = TTraits::GenerateRandomMessage(212);

		// Act:
		AssertCanBuildTransfer(
				message.size(),
				[&message](auto& builder) {
					TTraits::SetMessage(builder, message);
				},
				[&message](const auto& transfer) {
					// Assert: a message is present
					auto expectedMessageSize = message.size();
					auto actualMessageSize = transfer.MessageSize;
					EXPECT_EQ(expectedMessageSize, actualMessageSize);
					EXPECT_EQ(0, memcmp(message.data(), transfer.MessagePtr(), message.size()));

					// - no mosaics are present
					EXPECT_EQ(0u, transfer.MosaicsCount);
				});
	}

	MESSAGE_TEST(CannotSetEmptyMessage) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			// Assert:
			EXPECT_THROW(TTraits::SetMessage(builder, {}), catapult_invalid_argument);
		});
	}

	MESSAGE_TEST(CannotSetMultipleMessages) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			// Act:
			TTraits::SetMessage(builder, TTraits::GenerateRandomMessage(212));

			// Assert:
			EXPECT_THROW(TTraits::SetMessage(builder, TTraits::GenerateRandomMessage(212)), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, CannotSetMultipleMessages_DifferentTypes) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			auto message1 = test::GenerateRandomVector(123);
			auto message2 = test::GenerateRandomString(212);

			// Act:
			builder.setMessage(message1);

			// Assert:
			EXPECT_THROW(builder.setStringMessage(message2), catapult_runtime_error);
		});
	}

	// endregion

	// region mosaics

	namespace {
		std::map<MosaicId, Amount> ExtractMosaicsMap(const model::TransferTransaction& transfer) {
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

#define MOSAICS_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Id) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicIdTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Name) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicNameTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		template<typename TTraits>
		void AssertCanCreateTransferWithMosaics(size_t numMosaics) {
			// Arrange:
			auto mosaics = TTraits::GenerateMosaics(numMosaics);

			// Act:
			AssertCanBuildTransfer(
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
						const auto& expectedMosaics = TTraits::ToMosaicsMap(mosaics);
						const auto& actualMosaics = ExtractMosaicsMap(transfer);
						EXPECT_EQ(expectedMosaics, actualMosaics);
					});
		}
	}

	MOSAICS_TEST(CanCreateTransferWithSingleMosaic) {
		// Assert:
		AssertCanCreateTransferWithMosaics<TTraits>(1);
	}

	MOSAICS_TEST(CanCreateTransferWithMultipleMosaics) {
		// Assert:
		AssertCanCreateTransferWithMosaics<TTraits>(3);
	}

	MOSAICS_TEST(CannotAddSameMosaicMultipleTimes) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			auto mosaic = *TTraits::GenerateMosaics(1).cbegin();

			// Act:
			builder.addMosaic(mosaic.first, mosaic.second);

			// Assert:
			EXPECT_THROW(builder.addMosaic(mosaic.first, mosaic.second), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, CannotAddSameMosaicMultipleTimes_DifferentIdTypes) {
		// Arrange:
		RunBuilderTest([](auto& builder) {
			// Act:
			builder.addMosaic(Xem_Id, Amount(123));

			// Assert:
			EXPECT_THROW(builder.addMosaic("nem:xem", Amount(234)), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, MultipleMosaicsAreSortedByMosaicId) {
		// Arrange:
		AssertCanBuildTransfer(
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

	TEST(TEST_CLASS, CanCreateTransferWithMessageAndMosaics) {
		// Arrange:
		auto message = std::string("this is a great transfer!");

		// Act:
		AssertCanBuildTransfer(
				message.size() + 2 * sizeof(model::Mosaic),
				[&message](auto& builder) {
					builder.addMosaic(MosaicId(0), Amount(4'321));
					builder.setStringMessage(message);
					builder.addMosaic("nem:xem", Amount(1'000'000));
				},
				[&message](const auto& transfer) {
					// Assert: a message is present
					EXPECT_EQ(message.size(), transfer.MessageSize);
					EXPECT_EQ(0, memcmp(message.data(), transfer.MessagePtr(), message.size()));

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
