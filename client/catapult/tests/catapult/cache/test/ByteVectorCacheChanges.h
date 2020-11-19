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
#include "catapult/cache/CacheChanges.h"
#include "catapult/io/PodIoUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	// region byte vector cache changes

	/// Memory cache changes for a cache containing byte vector data.
	using ByteVectorCacheChanges = cache::MemoryCacheChangesT<std::vector<uint8_t>>;

	/// Creates a copy of \a changes.
	inline std::unique_ptr<ByteVectorCacheChanges> CopyByteVectorCacheChanges(const ByteVectorCacheChanges& changes) {
		auto pChangesCopy = std::make_unique<ByteVectorCacheChanges>();
		pChangesCopy->Added = changes.Added;
		pChangesCopy->Removed = changes.Removed;
		pChangesCopy->Copied = changes.Copied;
		return pChangesCopy;
	}

	/// Stubbed out byte vector cache delta.
	class ByteVectorCacheDelta {
	private:
		using ReturnType = std::unordered_set<const std::vector<uint8_t>*>;

	public:
		ReturnType addedElements() const {
			return {};
		}

		ReturnType modifiedElements() const {
			return {};
		}

		ReturnType removedElements() const {
			return {};
		}
	};

	/// Single cache changes for a cache containing byte vector data (using a stubbed out cache delta).
	using ByteVectorSingleCacheChanges = cache::SingleCacheChangesT<ByteVectorCacheDelta, std::vector<uint8_t>>;

	// endregion

	// region ByteVectorSerializer

	/// Byte vector serializer.
	struct ByteVectorSerializer {
		static void Save(const std::vector<uint8_t>& buffer, io::OutputStream& output) {
			io::Write64(output, buffer.size());
			output.write(buffer);
		}

		static std::vector<uint8_t> Load(io::InputStream& input) {
			std::vector<uint8_t> buffer(io::Read64(input));
			input.read(buffer);
			return buffer;
		}
	};

	// endregion

	// region ByteVectorBufferWriter

	/// Provides simplified interface for writing to a pre-allocated byte vector.
	class ByteVectorBufferWriter {
	public:
		/// Creates a writer that writes to \a buffer.
		explicit ByteVectorBufferWriter(std::vector<uint8_t>& buffer) : m_stream(buffer)
		{}

	public:
		/// Writes a 64-bit integer \a value.
		void write64(uint64_t value) {
			io::Write64(m_stream, value);
		}

		/// Writes a random buffer with specified \a size.
		std::vector<uint8_t> writeBuffer(size_t size) {
			write64(size);

			auto buffer = GenerateRandomVector(size);
			m_stream.write(buffer);
			return buffer;
		}

	private:
		mocks::MockMemoryStream m_stream;
	};

	// endregion

	// region ByteVectorBufferReader

	/// Provides simplified interface for reading from a pre-allocated byte vector.
	class ByteVectorBufferReader {
	public:
		/// Creates a reader that reads from \a buffer.
		explicit ByteVectorBufferReader(const std::vector<uint8_t>& buffer) : m_stream(const_cast<std::vector<uint8_t>&>(buffer))
		{}

	public:
		/// Reads a 64-bit integer value.
		uint64_t read64() {
			return io::Read64(m_stream);
		}

		/// Writes a size-prefixed buffer.
		std::vector<uint8_t> readBuffer() {
			auto size = read64();

			std::vector<uint8_t> buffer(size);
			m_stream.read(buffer);
			return buffer;
		}

	private:
		mocks::MockMemoryStream m_stream;
	};

	/// Asserts that the next buffers read from \a reader are equivalent to \a expectedBuffers (order isn't important)
	/// with optional \a message.
	inline void AssertEquivalent(
			const std::vector<std::vector<uint8_t>>& expectedBuffers,
			ByteVectorBufferReader& reader,
			const std::string& message = "") {
		std::set<std::vector<uint8_t>> expectedBuffersSet;
		std::set<std::vector<uint8_t>> readBuffersSet;
		for (auto i = 0u; i < expectedBuffers.size(); ++i) {
			expectedBuffersSet.emplace(expectedBuffers[i]);
			readBuffersSet.emplace(reader.readBuffer());
		}

		EXPECT_EQ(expectedBuffersSet, readBuffersSet) << message;
	}

	// endregion
}}
