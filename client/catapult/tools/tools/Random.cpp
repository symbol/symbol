#include "Random.h"
#include <random>

namespace catapult { namespace tools {

	namespace {
		class RandomGenerator {
		public:
			RandomGenerator() {
				std::random_device rd;
				auto seed = (static_cast<uint64_t>(rd()) << 32) | rd();
				m_gen.seed(seed);
			}

		public:
			static RandomGenerator& instance() {
				static RandomGenerator generator;
				return generator;
			}

			uint64_t operator()() {
				return m_gen();
			}

		private:
			std::mt19937_64 m_gen;
		};
	}

	uint64_t Random() {
		return RandomGenerator::instance()();
	}

	uint8_t RandomByte() {
		return static_cast<uint8_t>(Random());
	}
}}
