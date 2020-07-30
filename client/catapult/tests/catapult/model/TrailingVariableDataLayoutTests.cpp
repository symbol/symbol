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

#include "catapult/model/TrailingVariableDataLayout.h"
#include "catapult/model/Mosaic.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/VariableSizedEntityTestUtils.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TrailingVariableDataLayoutTests

	// region MosaicContainer

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

#pragma pack(pop)
	}

	// endregion

	// region size + alignment

#define LAYOUT_FIELDS FIELD(Size)

	TEST(TEST_CLASS, LayoutHasExpectedSize) {
		// Arrange:
		using Layout = TrailingVariableDataLayout<MosaicContainer, Mosaic>;
		auto expectedSize = 0u;

#define FIELD(X) expectedSize += SizeOf32<decltype(Layout::X)>();
		LAYOUT_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(Layout));
		EXPECT_EQ(4u, sizeof(Layout));
	}

	TEST(TEST_CLASS, LayoutHasProperAlignment) {
		using Layout = TrailingVariableDataLayout<MosaicContainer, Mosaic>;

#define FIELD(X) EXPECT_ALIGNED(Layout, X);
		LAYOUT_FIELDS
#undef FIELD
	}

#undef LAYOUT_FIELDS

	// endregion

	// region attachment pointer tests

	namespace {
		struct MosaicContainerTraits {
			static auto GenerateEntityWithAttachments(uint16_t count) {
				uint32_t entitySize = SizeOf32<MosaicContainer>() + count * SizeOf32<Mosaic>();
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

	// endregion
}}
