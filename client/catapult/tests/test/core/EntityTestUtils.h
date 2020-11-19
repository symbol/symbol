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

#pragma once
#include "catapult/model/EntityRange.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/preprocessor.h"
#include "tests/TestHarness.h"
#include <memory>
#include <vector>

namespace catapult { namespace test {

	/// Makes a random entity of \a size.
	template<typename TEntity = model::VerifiableEntity>
	std::shared_ptr<TEntity> CreateRandomEntityWithSize(uint32_t size) {
		using NonConstEntityType = std::remove_const_t<TEntity>;
		auto pEntity = utils::MakeUniqueWithSize<NonConstEntityType>(size);
		pEntity->Size = size;

		auto headerSize = model::VerifiableEntity::Header_Size;
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pEntity.get()) + headerSize, size - headerSize });
		return PORTABLE_MOVE(pEntity);
	}

	/// Creates a copy of a verifiable \a entity.
	template<typename T>
	std::unique_ptr<T> CopyEntity(const T& entity) {
		auto pEntity = utils::MakeUniqueWithSize<T>(entity.Size);
		std::memcpy(static_cast<void*>(pEntity.get()), &entity, entity.Size);
		return pEntity;
	}

	/// Copies \a entities into an entity range.
	template<typename T>
	model::EntityRange<T> CreateEntityRange(const std::vector<const T*>& entities) {
		std::vector<uint8_t> buffer;
		std::vector<size_t> offsets;
		for (const auto& pEntity : entities) {
			auto start = buffer.size();
			buffer.resize(start + pEntity->Size);
			std::memcpy(&buffer[start], pEntity, pEntity->Size);
			offsets.push_back(start);
		}

		return model::EntityRange<T>::CopyVariable(buffer.data(), buffer.size(), offsets);
	}

	/// Produces all entities from \a producer and adds them to a vector.
	template<typename TProducer>
	auto ProduceAll(const TProducer& producer) {
		std::vector<decltype(producer())> entities;
		for (;;) {
			auto pEntity = producer();
			if (!pEntity)
				break;

			entities.push_back(pEntity);
		}

		return entities;
	}

	/// Calculates the total size of all \a entities.
	template<typename T>
	uint32_t TotalSize(const std::vector<std::shared_ptr<T>>& entities) {
		uint32_t totalSize = 0;
		for (const auto& pEntity : entities)
			totalSize += pEntity->Size;

		return totalSize;
	}

	/// Asserts that \a expectedRange is equal to \a actualRange and outputs \a message if not.
	template<typename TEntity>
	void AssertEqualRange(
			const model::EntityRange<TEntity>& expectedRange,
			const model::EntityRange<TEntity>& actualRange,
			const char* message) {
		ASSERT_EQ(expectedRange.size(), actualRange.size());
		auto iter = expectedRange.cbegin();
		auto i = 0u;
		for (const auto& entity : actualRange) {
			EXPECT_EQ(*iter++, entity) << message << " at " << i;
			++i;
		}
	}
}}
