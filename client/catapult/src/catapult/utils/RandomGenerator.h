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
#include <random>

namespace catapult { namespace utils {

	// region HighEntropyRandomGenerator

	/// Generator for high entropy random numbers.
	class HighEntropyRandomGenerator {
	public:
		using result_type = uint64_t;

	public:
		/// Creates the generator.
		HighEntropyRandomGenerator();

		/// Creates the generator using the specified \a token source.
		explicit HighEntropyRandomGenerator(const std::string& token);

	public:
		/// Gets the mininmum generated value.
		static constexpr result_type min() {
			return std::random_device::min();
		}

		/// Gets the maximum generated value.
		static constexpr result_type max() {
			return (static_cast<uint64_t>(std::random_device::max()) << 32) | std::random_device::max();
		}

	public:
		/// Generates a random value.
		result_type operator()();

		/// Generates \a count random bytes into \a pOut.
		void fill(uint8_t* pOut, size_t count);

	private:
		std::random_device m_rd;
	};

	// endregion

	// region LowEntropyRandomGenerator

	/// Generator for low entropy random numbers.
	class LowEntropyRandomGenerator {
	public:
		using result_type = uint64_t;

	public:
		/// Creates the generator.
		LowEntropyRandomGenerator();

	public:
		/// Gets the mininmum generated value.
		static constexpr result_type min() {
			return std::mt19937_64::min();
		}

		/// Gets the maximum generated value.
		static constexpr result_type max() {
			return std::mt19937_64::max();
		}

	public:
		/// Generates a random value.
		result_type operator()();

		/// Generates \a count random bytes into \a pOut.
		void fill(uint8_t* pOut, size_t count);

	private:
		std::mt19937_64& m_gen;
	};

	// endregion
}}
