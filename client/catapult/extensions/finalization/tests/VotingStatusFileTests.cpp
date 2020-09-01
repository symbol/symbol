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

#include "finalization/src/VotingStatusFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace finalization {

#define TEST_CLASS VotingStatusFileTests

	namespace {
		// region asserts

		constexpr size_t Expected_File_Size = sizeof(uint64_t) + 2 * sizeof(uint8_t);

		void AssertNotExists(const std::string& filename) {
			EXPECT_FALSE(boost::filesystem::exists(filename));
		}

		void AssertExists(const std::string& filename) {
			EXPECT_TRUE(boost::filesystem::exists(filename));
			EXPECT_EQ(Expected_File_Size, io::RawFile(filename, io::OpenMode::Read_Only).size());
		}

		void AssertEqual(const chain::VotingStatus& expected, const chain::VotingStatus& actual) {
			EXPECT_EQ(expected.Point, actual.Point);
			EXPECT_EQ(expected.HasSentPrevote, actual.HasSentPrevote);
			EXPECT_EQ(expected.HasSentPrecommit, actual.HasSentPrecommit);
		}

		// endregion
	}

	// region constructor / load

	TEST(TEST_CLASS, ConstructorDoesNotCreateFile) {
		// Act:
		test::TempFileGuard tempFile("foo.dat");
		VotingStatusFile votingStatusFile(tempFile.name());

		// Assert:
		AssertNotExists(tempFile.name());
	}

	TEST(TEST_CLASS, LoadDoesNotCreateFile) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		VotingStatusFile votingStatusFile(tempFile.name());

		// Act:
		auto status = votingStatusFile.load();

		// Assert:
		AssertNotExists(tempFile.name());
		AssertEqual({ FinalizationPoint(1), false, false }, status);
	}

	TEST(TEST_CLASS, CanLoadSavedValue) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		VotingStatusFile votingStatusFile(tempFile.name());

		{
			io::RawFile rawFile(tempFile.name(), io::OpenMode::Read_Write);
			rawFile.write(std::vector<uint8_t>{ 7, 0, 0, 0, 0, 0, 0, 0, 0, 8 });

			// Sanity:
			EXPECT_EQ(Expected_File_Size, rawFile.size());
		}

		// Act:
		auto status = votingStatusFile.load();

		// Assert:
		AssertEqual({ FinalizationPoint(7), false, true }, status);
	}

	// endregion

	// region save

	TEST(TEST_CLASS, CanSaveValue) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		VotingStatusFile votingStatusFile(tempFile.name());

		// Act:
		votingStatusFile.save({ FinalizationPoint(17), false, true });

		// Assert:
		AssertExists(tempFile.name());
		AssertEqual({ FinalizationPoint(17), false, true }, votingStatusFile.load());
	}

	TEST(TEST_CLASS, CanResetValue) {
		// Arrange:
		test::TempFileGuard tempFile("foo.dat");
		VotingStatusFile votingStatusFile(tempFile.name());

		// Act:
		votingStatusFile.save({ FinalizationPoint(17), false, true });
		votingStatusFile.save({ FinalizationPoint(82), true, false });

		// Assert:
		AssertExists(tempFile.name());
		AssertEqual({ FinalizationPoint(82), true, false }, votingStatusFile.load());
	}

	// endregion
}}
