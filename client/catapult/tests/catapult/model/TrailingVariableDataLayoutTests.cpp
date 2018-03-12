#include "catapult/model/TrailingVariableDataLayout.h"
#include "catapult/model/Mosaic.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TrailingVariableDataLayoutTests

	namespace {
#pragma pack(push, 1)

		struct MosaicContainer : public TrailingVariableDataLayout<MosaicContainer, Mosaic> {
		public:
			// fixed size header
			uint16_t MosaicsCount;

		public:
			const Mosaic* MosaicsPtr() const {
				return MosaicsCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
			}

			Mosaic* MosaicsPtr() {
				return MosaicsCount ? ToTypedPointer(PayloadStart(*this)) : nullptr;
			}

		public:
			static constexpr uint64_t CalculateRealSize(const MosaicContainer& container) noexcept {
				return sizeof(MosaicContainer) + container.MosaicsCount * sizeof(Mosaic);
			}
		};

#pragma pack(push, 1)
	}

	TEST(TEST_CLASS, LayoutHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(uint32_t); // size

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(TrailingVariableDataLayout<MosaicContainer, Mosaic>));
		EXPECT_EQ(4u, sizeof(TrailingVariableDataLayout<MosaicContainer, Mosaic>));
	}

	namespace {
		struct MosaicContainerTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				uint32_t entitySize = sizeof(MosaicContainer) + count * sizeof(Mosaic);
				auto pContainer = utils::MakeUniqueWithSize<MosaicContainer>(entitySize);
				pContainer->Size = entitySize;
				pContainer->MosaicsCount = count;
				return pContainer;
			}

			template<typename TEntity>
			static auto GetAttachmentPointer(TEntity& entity) {
				return entity.MosaicsPtr();
			}
		};
	}

	DEFINE_ATTACHMENT_POINTER_TESTS(TEST_CLASS, MosaicContainerTraits) // MosaicsPtr
}}
