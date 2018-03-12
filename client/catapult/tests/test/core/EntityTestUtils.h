#pragma once
#include "catapult/model/EntityRange.h"
#include "catapult/model/VerifiableEntity.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/TestHarness.h"
#include <memory>
#include <vector>

namespace catapult { namespace test {

	/// Creates a copy of a verifiable \a entity.
	template<typename T>
	std::unique_ptr<T> CopyEntity(const T& entity) {
		auto pEntity = utils::MakeUniqueWithSize<T>(entity.Size);
		std::memcpy(pEntity.get(), &entity, entity.Size);
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
