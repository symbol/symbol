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

#include "Random.h"
#include "catapult/utils/RandomGenerator.h"
#include <algorithm>

namespace catapult { namespace tools {

	uint64_t Random() {
		return utils::LowEntropyRandomGenerator()();
	}

	uint8_t RandomByte() {
		return static_cast<uint8_t>(Random());
	}

	std::vector<uint8_t> GenerateRandomVector(size_t size) {
		std::vector<uint8_t> container;
		container.resize(size);
		std::generate_n(container.begin(), container.size(), RandomByte);
		return container;
	}
}}
