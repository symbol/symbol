#include "Random.h"
#include <random>

namespace catapult { namespace test {

	namespace {
		template<typename T>
		void RandomFill(T& container) {
			std::generate_n(container.begin(), container.size(), []() { return static_cast<typename T::value_type>(Random()); });
		}

		template<typename T>
		T GenerateRandomContainer(size_t size) {
			T container;
			container.resize(size);
			RandomFill(container);
			return container;
		}

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

	std::string GenerateRandomString(size_t size) {
		return GenerateRandomContainer<std::string>(size);
	}

	std::string GenerateRandomHexString(size_t size) {
		std::string str;
		str.resize(size);
		std::generate_n(str.begin(), str.size(), []() {
			auto value = Random() % 16;
			return static_cast<char>(value < 10 ? (value + '0') : (value - 10 + 'a'));
		});
		return str;
	}

	std::vector<uint8_t> GenerateRandomVector(size_t size) {
		return GenerateRandomContainer<std::vector<uint8_t>>(size);
	}

	void FillWithRandomData(std::vector<uint8_t>& vec) {
		RandomFill(vec);
	}

	void FillWithRandomData(const MutableRawBuffer& dataBuffer) {
		std::generate_n(dataBuffer.pData, dataBuffer.Size, RandomByte);
	}
}}
