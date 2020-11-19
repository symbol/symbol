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
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/FileQueue.h"
#include "catapult/io/IndexFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <filesystem>

namespace catapult { namespace test {

	// region test context

	/// Test context used for message ingestion tests.
	class MessageIngestionTestContext {
	public:
		uint64_t readIndexReaderFile(const std::string& queueName, const std::string& indexFilename = "index_broker_r.dat") const {
			auto queuePath = qualifyQueueName(queueName);
			auto indexFile = io::IndexFile((queuePath / indexFilename).generic_string());
			return indexFile.exists() ? indexFile.get() : 0;
		}

		size_t countIndexFiles(const std::string& queueName) const {
			auto numIndexFiles = 0u;
			auto queuePath = qualifyQueueName(queueName);
			for (const auto& indexFilename : { "index_server.dat", "index.dat", "index_broker_r.dat", "index_server_r.dat" }) {
				if (std::filesystem::exists(queuePath / indexFilename))
					++numIndexFiles;
			}

			return numIndexFiles;
		}

		size_t countMessageFiles(const std::string& queueName) const {
			auto queuePath = qualifyQueueName(queueName);
			auto begin = std::filesystem::directory_iterator(queuePath);
			auto end = std::filesystem::directory_iterator();
			auto numMessages = static_cast<size_t>(std::distance(begin, end));

			// subtract index files
			return numMessages - countIndexFiles(queueName);
		}

		void writeMessages(const std::string& queueName, size_t numMessages, const consumer<io::OutputStream&>& messageWriter) {
			// need to create containing subdirectory before writing
			config::CatapultDataDirectoryPreparer::Prepare(dataDirectory().rootDir().path());

			auto queuePath = qualifyQueueName(queueName);
			io::FileQueueWriter writer(queuePath.generic_string());

			for (auto i = 0u; i < numMessages; ++i) {
				messageWriter(writer);
				writer.flush();
			}
		}

	protected:
		config::CatapultDataDirectory dataDirectory() const {
			return config::CatapultDataDirectory(m_tempDir.name());
		}

		std::string resourcesDirectory() const {
			return dataDirectory().rootDir().file("resources");
		}

	private:
		std::filesystem::path qualifyQueueName(const std::string& queueName) const {
			return dataDirectory().spoolDir(queueName).path();
		}

	private:
		TempDirectoryGuard m_tempDir;
	};

	// endregion

	// region utils

	/// Writes \a numMessages using \a context given traits.
	template<typename TTraits>
	void WriteMessages(MessageIngestionTestContext& context, size_t numMessages) {
		context.writeMessages(TTraits::Queue_Directory_Name, numMessages, TTraits::WriteMessage);
	}

	/// Produces and consumes \a numMessages starting at \a startIndex using \a context given traits.
	/// Queue is expected to contain \a numExpectedIndexFiles index files after boot.
	template<typename TTraits, typename TMessageIngestionTestContext>
	void ProduceAndConsumeMessages(
			TMessageIngestionTestContext& context,
			size_t numMessages,
			size_t startIndex = 0,
			size_t numExpectedIndexFiles = 2) {
		// Arrange:
		WriteMessages<TTraits>(context, numMessages);

		// Sanity:
		EXPECT_EQ(numMessages, context.countMessageFiles(TTraits::Queue_Directory_Name));
		EXPECT_EQ(startIndex, context.readIndexReaderFile(TTraits::Queue_Directory_Name));

		// Act:
		context.boot();

		// Assert:
		WAIT_FOR_ZERO_EXPR(context.countMessageFiles(TTraits::Queue_Directory_Name));
		EXPECT_EQ(numExpectedIndexFiles, context.countIndexFiles(TTraits::Queue_Directory_Name));
		EXPECT_EQ(startIndex + numMessages, context.readIndexReaderFile(TTraits::Queue_Directory_Name));
	}

	// endregion
}}
