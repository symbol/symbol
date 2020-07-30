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

#include "RandomGenerator.h"
#include <cstring>

namespace catapult { namespace utils {

	namespace {
		template<typename TGenerator>
		void Fill(uint8_t* pOut, size_t count, TGenerator& generator) {
			using GeneratorResultType = decltype(generator());

			for (size_t i = 0; i < count; i += sizeof(GeneratorResultType)) {
				auto randomValue = generator();
				std::memcpy(pOut + i, &randomValue, std::min(sizeof(GeneratorResultType), count - i));
			}
		}
	}

	// region HighEntropyRandomGenerator

	HighEntropyRandomGenerator::HighEntropyRandomGenerator() = default;

	HighEntropyRandomGenerator::HighEntropyRandomGenerator(const std::string& token) : m_rd(token)
	{}

	HighEntropyRandomGenerator::result_type HighEntropyRandomGenerator::operator()() {
		return (static_cast<uint64_t>(m_rd()) << 32) | m_rd();
	}

	void HighEntropyRandomGenerator::fill(uint8_t* pOut, size_t count) {
		Fill(pOut, count, m_rd);
	}

	// endregion

	// region LowEntropyRandomGenerator

	namespace {
		std::mt19937_64& GetThreadLocalLowEntropyRandomGenerator() {
			thread_local auto gen = std::mt19937_64(HighEntropyRandomGenerator()());
			return gen;
		}
	}

	LowEntropyRandomGenerator::LowEntropyRandomGenerator() : m_gen(GetThreadLocalLowEntropyRandomGenerator())
	{}

	LowEntropyRandomGenerator::result_type LowEntropyRandomGenerator::operator()() {
		return m_gen();
	}

	void LowEntropyRandomGenerator::fill(uint8_t* pOut, size_t count) {
		Fill(pOut, count, m_gen);
	}

	// endregion
}}
