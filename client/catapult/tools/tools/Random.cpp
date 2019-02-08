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

#include "Random.h"

namespace catapult { namespace tools {

	// region random generator

	RandomGenerator::RandomGenerator() {
		std::random_device rd;
		auto seed = (static_cast<uint64_t>(rd()) << 32) | rd();
		m_gen.seed(seed);
	}

	RandomGenerator::RandomGenerator(uint64_t seed) : m_gen(seed)
	{}

	uint64_t RandomGenerator::operator()() {
		return m_gen();
	}

	// endregion

	uint64_t Random() {
		static RandomGenerator generator;
		return generator();
	}

	uint8_t RandomByte() {
		return static_cast<uint8_t>(Random());
	}
}}
