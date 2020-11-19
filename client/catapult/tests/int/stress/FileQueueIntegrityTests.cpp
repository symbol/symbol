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

#include "catapult/io/FileQueue.h"
#include "catapult/thread/ThreadGroup.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace io {

#define TEST_CLASS FileQueueIntegrityTests

	namespace {
		using BufferVector = std::vector<std::vector<uint8_t>>;

		uint64_t GetNumIterations() {
			return test::GetStressIterationCount() ? 5'000 : 500;
		}

		// region QueueTestContext

		class QueueTestContext {
		public:
			QueueTestContext()
					: m_tempDataDir("q")
					, m_directory(m_tempDataDir.name())
			{}

		public:
			FileQueueWriter createWriter() {
				return FileQueueWriter(m_tempDataDir.name());
			}

			FileQueueReader createReader() {
				return FileQueueReader(m_tempDataDir.name());
			}

		public:
			uint64_t readIndexFile(const std::string& name) const {
				return IndexFile((m_directory / name).generic_string()).get();
			}

			size_t countFiles() const {
				auto begin = std::filesystem::directory_iterator(m_directory);
				auto end = std::filesystem::directory_iterator();
				return static_cast<size_t>(std::distance(begin, end));
			}

			bool exists(const std::string& name) const {
				return std::filesystem::exists(m_directory / name);
			}

		private:
			test::TempDirectoryGuard m_tempDataDir;
			std::filesystem::path m_directory;
		};

		// endregion

		// region test utils

		BufferVector GenerateRandomBuffers(size_t count) {
			BufferVector buffers;
			for (auto i = 0u; i < count; ++i)
				buffers.push_back(test::GenerateRandomVector(test::Random() % 256));

			return buffers;
		}

		void WriteAll(OutputStream& writer, const BufferVector& writeBuffers) {
			for (const auto& buffer : writeBuffers) {
				writer.write(buffer);
				writer.flush();
			}
		}

		BufferVector ReadAll(FileQueueReader& reader, size_t count) {
			BufferVector readBuffers;
			while (readBuffers.size() < count) {
				bool shouldContinue = true;
				while (shouldContinue) {
					shouldContinue = reader.tryReadNextMessage([&readBuffers](const auto& buffer) {
						readBuffers.push_back(buffer);
					});
				}
			}

			return readBuffers;
		}

		void AssertEqualBufferVectors(const BufferVector& expected, const BufferVector& actual) {
			ASSERT_EQ(expected.size(), actual.size());

			for (auto i = 0u; i < expected.size(); ++i)
				EXPECT_EQ(expected[i], actual[i]) << "buffer at " << i;
		}

		void AssertOnlyIndexFilesRemain(const QueueTestContext& context, uint64_t expectedValue) {
			EXPECT_EQ(2u, context.countFiles());
			EXPECT_TRUE(context.exists("index.dat"));
			EXPECT_TRUE(context.exists("index_reader.dat"));

			EXPECT_EQ(expectedValue, context.readIndexFile("index.dat"));
			EXPECT_EQ(expectedValue, context.readIndexFile("index_reader.dat"));
		}

		// endregion
	}

	// region single instance

	TEST(TEST_CLASS, CanProcessSequentially) {
		// Arrange:
		QueueTestContext context;
		auto writeBuffers = GenerateRandomBuffers(GetNumIterations());

		// Act: write all files
		auto writer = context.createWriter();
		WriteAll(writer, writeBuffers);

		// Act: read all files
		auto reader = context.createReader();
		auto readBuffers = ReadAll(reader, GetNumIterations());

		// Assert: all buffers were read and only index files remain
		AssertEqualBufferVectors(writeBuffers, readBuffers);
		AssertOnlyIndexFilesRemain(context, GetNumIterations());
	}

	TEST(TEST_CLASS, CanProcessInParallelWithProducerThreadAndConsumerThread) {
		// Arrange:
		QueueTestContext context;
		auto writeBuffers = GenerateRandomBuffers(GetNumIterations());

		// Act: writer thread
		thread::ThreadGroup threads;
		auto writer = context.createWriter();
		threads.spawn([&writeBuffers, &writer] {
			WriteAll(writer, writeBuffers);
		});

		// - reader thread
		BufferVector readBuffers;
		auto reader = context.createReader();
		threads.spawn([&readBuffers, &reader] {
			readBuffers = ReadAll(reader, GetNumIterations());
		});

		// - wait for all threads
		threads.join();

		// Assert: all buffers were read and only index files remain
		AssertEqualBufferVectors(writeBuffers, readBuffers);
		AssertOnlyIndexFilesRemain(context, GetNumIterations());
	}

	// endregion

	// region multiple instances

	namespace {
		void WriteAll(QueueTestContext& context, const BufferVector& writeBuffers) {
			// this helper is used by multi-instance tests and creates new writer for each write
			for (const auto& buffer : writeBuffers) {
				auto writer = context.createWriter();
				writer.write(buffer);
				writer.flush();
			}
		}

		BufferVector ReadAll(QueueTestContext& context, size_t count) {
			// this helper is used by multi-instance tests and creates new reader for each read
			BufferVector readBuffers;
			while (readBuffers.size() < count) {
				bool shouldContinue = true;
				auto reader = context.createReader();
				while (shouldContinue) {
					shouldContinue = reader.tryReadNextMessage([&readBuffers](const auto& buffer) {
						readBuffers.push_back(buffer);
					});
				}
			}

			return readBuffers;
		}
	}

	TEST(TEST_CLASS, CanProcessSequentially_MultipleInstances) {
		// Arrange:
		QueueTestContext context;
		auto writeBuffers = GenerateRandomBuffers(GetNumIterations());

		// Act: write all files
		WriteAll(context, writeBuffers);

		// Act: read all files
		auto readBuffers = ReadAll(context, GetNumIterations());

		// Assert: all buffers were read and only index files remain
		AssertEqualBufferVectors(writeBuffers, readBuffers);
		AssertOnlyIndexFilesRemain(context, GetNumIterations());
	}

	TEST(TEST_CLASS, CanProcessInParallelWithProducerThreadAndConsumerThread_MultipleInstances) {
		// Arrange:
		QueueTestContext context;
		auto writeBuffers = GenerateRandomBuffers(GetNumIterations());

		// Act: writer thread
		thread::ThreadGroup threads;
		threads.spawn([&context, &writeBuffers] {
			WriteAll(context, writeBuffers);
		});

		// - reader thread
		BufferVector readBuffers;
		threads.spawn([&context, &readBuffers] {
			readBuffers = ReadAll(context, GetNumIterations());
		});

		// - wait for all threads
		threads.join();

		// Assert: all buffers were read and only index files remain
		AssertEqualBufferVectors(writeBuffers, readBuffers);
		AssertOnlyIndexFilesRemain(context, GetNumIterations());
	}

	// endregion
}}
