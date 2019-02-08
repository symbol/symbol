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

#include "catapult/crypto/Signer.h"
#include "tests/test/nodeps/Random.h"
#include <benchmark/benchmark.h>

namespace catapult { namespace crypto {

	namespace {
		void BenchmarkVerify(benchmark::State& state) {
			Key key;
			std::vector<uint8_t> data(279);
			Signature signature;
			for (auto _ : state) {
				state.PauseTiming();
				test::FillWithRandomData(key);
				test::FillWithRandomData(data);
				test::FillWithRandomData(signature);
				state.ResumeTiming();

				crypto::Verify(key, data, signature);
			}

			state.SetBytesProcessed(static_cast<int64_t>(data.size() * state.iterations()));
		}

		void RegisterTests() {
			benchmark::RegisterBenchmark("BenchmarkVerify", BenchmarkVerify)
					->UseRealTime()
					->Threads(1)
					->Threads(2)
					->Threads(4)
					->Threads(8);
		}
	}
}}

int main(int argc, char **argv) {
	catapult::crypto::RegisterTests();
	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();
}
