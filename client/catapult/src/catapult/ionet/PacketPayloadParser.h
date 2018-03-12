#pragma once
#include "catapult/utils/Logging.h"
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace ionet {

	namespace detail {
		template<typename TEntity, typename TIsValidPredicate>
		bool IsEntityValid(const TEntity& entity, TIsValidPredicate isValid) {
			if (isValid(entity))
				return true;

			CATAPULT_LOG(warning) << "entity (header size = " << sizeof(TEntity)
					<< ") has failed validity check with size " << entity.Size;
			return false;
		}
	}

	/// Extracts entity offsets from \a buffer with a validity check (\a isValid).
	/// \note If the buffer is invalid and/or contains partial entities, the returned vector will be empty.
	template<typename TEntity, typename TIsValidPredicate>
	std::vector<size_t> ExtractEntityOffsets(const RawBuffer& buffer, TIsValidPredicate isValid) {
		using ResultType = std::vector<size_t>;
		if (0 == buffer.Size)
			return ResultType();

#define LOG_EXCEEDS_REMAINING_BYTES(SIZE, MESSAGE) \
	CATAPULT_LOG(warning) << MESSAGE << " (" << SIZE << ") exceeds remaining bytes (" << remainingBytes << ")";

		std::vector<size_t> offsets;
		auto remainingBytes = buffer.Size;
		while (0 != remainingBytes) {
			if (sizeof(TEntity) > remainingBytes) {
				LOG_EXCEEDS_REMAINING_BYTES(sizeof(TEntity), "entity header size");
				return ResultType();
			}

			auto offset = buffer.Size - remainingBytes;
			const auto& entity = reinterpret_cast<const TEntity&>(*(buffer.pData + offset));
			if (!detail::IsEntityValid(entity, isValid))
				return ResultType();

			if (entity.Size > remainingBytes) {
				LOG_EXCEEDS_REMAINING_BYTES(entity.Size, "entity size");
				return ResultType();
			}

			offsets.push_back(offset);
			remainingBytes -= entity.Size;
		}

#undef LOG_EXCEEDS_REMAINING_BYTES

		return offsets;
	}

#define LOG_DATA_SIZE_ERROR(SIZE, MESSAGE) \
	CATAPULT_LOG(warning) << "buffer data size (" << buffer.Size << ") " << MESSAGE << " (" << SIZE << ")";

	/// Determines if \a buffer contains a single entity with a validity check (\a isValid).
	/// \note If the buffer is invalid and/or contains partial or multiple entities, \c false will be returned.
	template<typename TEntity, typename TIsValidPredicate>
	bool ContainsSingleEntity(const RawBuffer& buffer, TIsValidPredicate isValid) {
		if (0 == buffer.Size)
			return false;

		if (buffer.Size < sizeof(TEntity)) {
			LOG_DATA_SIZE_ERROR(sizeof(TEntity), "must be at least entity size");
			return false;
		}

		const auto& entity = reinterpret_cast<const TEntity&>(*buffer.pData);
		if (!detail::IsEntityValid(entity, isValid))
			return false;

		if (entity.Size != buffer.Size) {
			LOG_DATA_SIZE_ERROR(entity.Size, "is inconsistent with entity size");
			return false;
		}

		return true;
	}

	/// Counts the number of fixed size structures in \a buffer.
	/// \note If the buffer is invalid and/or contains partial structures, \c 0 will be returned.
	template<typename TStructure>
	size_t CountFixedSizeStructures(const RawBuffer& buffer) {
		if (0 == buffer.Size)
			return 0;

		constexpr auto Structure_Size = sizeof(TStructure);
		if (0 != buffer.Size % Structure_Size) {
			LOG_DATA_SIZE_ERROR(Structure_Size, "contains fractional structures of size");
			return 0;
		}

		return buffer.Size / Structure_Size;
	}

#undef LOG_DATA_SIZE_ERROR
}}
