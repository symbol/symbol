#include "catapult/model/SizePrefixedEntity.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS SizePrefixedEntityTests

	TEST(TEST_CLASS, EntityHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(uint32_t); // size

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(SizePrefixedEntity));
		EXPECT_EQ(4u, sizeof(SizePrefixedEntity));
	}

	// region data pointers

	namespace {
#pragma pack(push, 1)

		struct CustomSizePrefixedEntity : public SizePrefixedEntity {
		public:
			uint32_t Custom;

		public:
			auto StartPointer() const {
				return ToBytePointer();
			}

			auto EndPointer() const {
				return PayloadStart(*this);
			}

		public:
			auto StartPointer() {
				return ToBytePointer();
			}

			auto EndPointer() {
				return PayloadStart(*this);
			}

		public:
			static constexpr uint64_t CalculateRealSize(const CustomSizePrefixedEntity&) noexcept {
				return sizeof(CustomSizePrefixedEntity);
			}
		};

#pragma pack(pop)

		struct NonConstTraits {
			static auto ToStartPointer(CustomSizePrefixedEntity& entity) {
				return entity.StartPointer();
			}

			static auto ToEndPointer(CustomSizePrefixedEntity& entity) {
				return entity.EndPointer();
			}
		};

		struct ConstTraits {
			static auto ToStartPointer(const CustomSizePrefixedEntity& entity) {
				return entity.StartPointer();
			}

			static auto ToEndPointer(const CustomSizePrefixedEntity& entity) {
				return entity.EndPointer();
			}
		};
	}

#define BYTE_POINTER_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Const) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ConstTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonConst) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonConstTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	BYTE_POINTER_TEST(ToBytePointerReturnsPointerToStartOfEntity) {
		// Arrange:
		CustomSizePrefixedEntity entity;

		// Act:
		auto pEntityStart = test::AsVoidPointer(TTraits::ToStartPointer(entity));

		// Assert:
		EXPECT_EQ(&entity, pEntityStart);
	}

	BYTE_POINTER_TEST(PayloadStartReturnsPointerToStartOfEntityPayload_IfEntitySizeIsCorrect) {
		// Arrange:
		CustomSizePrefixedEntity entity;
		entity.Size = sizeof(CustomSizePrefixedEntity);

		// Act:
		auto pEntityEnd = test::AsVoidPointer(TTraits::ToEndPointer(entity));

		// Assert:
		EXPECT_EQ(&entity + 1, pEntityEnd);
	}

	namespace {
		template<typename TTraits>
		void AssertPayloadStartReturnsNullPointerForSize(uint32_t size) {
			// Arrange:
			CustomSizePrefixedEntity entity;
			entity.Size = size;

			// Act:
			auto pEntityEnd = test::AsVoidPointer(TTraits::ToEndPointer(entity));

			// Assert:
			EXPECT_FALSE(!!pEntityEnd);
		}
	}

	BYTE_POINTER_TEST(PayloadStartReturnsNullPointer_IfEntitySizeIsTooLarge) {
		// Assert:
		AssertPayloadStartReturnsNullPointerForSize<TTraits>(sizeof(CustomSizePrefixedEntity) + 1);
	}

	BYTE_POINTER_TEST(PayloadStartReturnsNullPointer_IfEntitySizeIsTooSmall) {
		// Assert:
		AssertPayloadStartReturnsNullPointerForSize<TTraits>(sizeof(CustomSizePrefixedEntity) - 1);
	}

	// endregion

	// region equality

	namespace {
		const char* Default_Key = "default";

		std::unique_ptr<SizePrefixedEntity> CreateSizePrefixedEntity(const std::vector<uint8_t>& payload) {
			auto numSizeBytes = sizeof(SizePrefixedEntity::Size);
			auto entitySize = numSizeBytes + payload.size();
			auto pEntity = utils::MakeUniqueWithSize<SizePrefixedEntity>(entitySize);

			std::memcpy(reinterpret_cast<uint8_t*>(pEntity.get()) + numSizeBytes, payload.data(), payload.size());
			pEntity->Size = static_cast<uint32_t>(entitySize);
			return pEntity;
		}

		std::vector<uint8_t> ResizePayload(const std::vector<uint8_t>& data, int delta) {
			auto copy = data;
			copy.resize(copy.size() + static_cast<size_t>(delta));
			return copy;
		}

		std::vector<uint8_t> ChangePayloadByte(const std::vector<uint8_t>& data, size_t offset) {
			auto copy = data;
			copy[offset] ^= 0xFF;
			return copy;
		}

		std::unordered_map<std::string, std::unique_ptr<SizePrefixedEntity>> GenerateEqualityInstanceMap() {
			std::unordered_map<std::string, std::unique_ptr<SizePrefixedEntity>> map;
			auto payload = test::GenerateRandomVector(123);

			map.emplace(Default_Key, CreateSizePrefixedEntity(payload));
			map.emplace("copy", CreateSizePrefixedEntity(payload));
			map.emplace("diff-size-gt", CreateSizePrefixedEntity(ResizePayload(payload, 1)));
			map.emplace("diff-size-lt", CreateSizePrefixedEntity(ResizePayload(payload, -1)));
			map.emplace("diff-data-first", CreateSizePrefixedEntity(ChangePayloadByte(payload, 0)));
			map.emplace("diff-data-middle", CreateSizePrefixedEntity(ChangePayloadByte(payload, payload.size() / 2)));
			map.emplace("diff-data-last", CreateSizePrefixedEntity(ChangePayloadByte(payload, payload.size() - 1)));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
