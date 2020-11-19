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

#include "catapult/io/EntityIoUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace io {

#define TEST_CLASS EntityIoUtilsTests

	namespace {
		struct CustomEntity : public model::SizePrefixedEntity {
			uint64_t Value;
		};

		std::unique_ptr<CustomEntity> CreateRandomEntity(uint32_t size) {
			auto pEntity = utils::MakeUniqueWithSize<CustomEntity>(size);
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pEntity.get()), size });
			pEntity->Size = size;
			return pEntity;
		}
	}

	TEST(TEST_CLASS, CanWriteEntity) {
		// Arrange:
		auto pEntity = CreateRandomEntity(49);

		std::vector<uint8_t> buffer;
		mocks::MockMemoryStream stream(buffer);

		// Act:
		WriteEntity(stream, *pEntity);

		// Assert: full entity was written
		ASSERT_EQ(pEntity->Size, buffer.size());
		EXPECT_EQ_MEMORY(pEntity.get(), buffer.data(), buffer.size());
	}

	TEST(TEST_CLASS, CanReadEntity) {
		// Arrange:
		auto pEntity = CreateRandomEntity(49);

		std::vector<uint8_t> buffer(pEntity->Size);
		std::memcpy(buffer.data(), pEntity.get(), pEntity->Size);
		mocks::MockMemoryStream stream(buffer);

		// Act:
		auto pReadEntity = ReadEntity<CustomEntity>(stream);

		// Assert: full entity was read
		EXPECT_EQ(*pEntity, *pReadEntity);
	}

	TEST(TEST_CLASS, CannotReadEntityWithInvalidSize) {
		// Arrange:
		auto pEntity = CreateRandomEntity(49);

		// - indicate entity extends one byte beyond end of stream
		std::vector<uint8_t> buffer(pEntity->Size);
		std::memcpy(buffer.data(), pEntity.get(), pEntity->Size);
		++reinterpret_cast<uint32_t&>(buffer[0]);
		mocks::MockMemoryStream stream(buffer);

		// Act + Assert:
		EXPECT_THROW(ReadEntity<CustomEntity>(stream), catapult_runtime_error);
	}
}}
