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
#include "tests/TestHarness.h"
#include <boost/algorithm/string/split.hpp>
#include <cctype>
#include <fstream>

namespace catapult { namespace test {

	/// Runs an input dependent test by loading input from \a sourceFilename, parsing each line with \a lineParser
	/// and passing all parsed lines to \a action.
	template<typename TLineParser, typename TAction>
	void RunInputDependentTest(const std::string& sourceFilename, TLineParser lineParser, TAction action) {
		auto i = 0u;
		std::ifstream input(sourceFilename);
		if (!input) {
			std::ostringstream out;
			out << "unable to load file '" << sourceFilename << "'";
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}

		for (std::string line; getline(input, line);) {
			if (line.empty() || '#' == line[0])
				continue;

			// - strip all spaces from the string
			line.erase(std::remove_if(line.begin(), line.end(), [](auto ch) { return std::isspace(ch); }), line.end());

			// - split the line on colons
			std::vector<std::string> parts;
			boost::algorithm::split(parts, line, [](auto ch) { return ':' == ch; });

			// - parse the line
			auto result = lineParser(parts);
			if (!result.second) {
				FAIL() << "malformed line: " << line;
				continue;
			}

			// Act + Assert:
			action(result.first);
			++i;
		}

		CATAPULT_LOG(debug) << "executed " << i << " test cases";
	}
}}
