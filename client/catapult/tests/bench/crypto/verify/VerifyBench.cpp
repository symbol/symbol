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

#include "catapult/crypto/Signer.h"
#include "catapult/utils/Logging.h"
#include "catapult/utils/RandomGenerator.h"
#include "tests/bench/nodeps/Random.h"
#include <benchmark/benchmark.h>

namespace catapult { namespace crypto {

	namespace {
		constexpr auto Data_Size = 279;

		auto CreateRandomKeyPair() {
			return KeyPair::FromPrivate(PrivateKey::Generate(bench::RandomByte));
		}

		RandomFiller CreateRandomFiller() {
			return [](auto* pOut, auto count) {
				utils::HighEntropyRandomGenerator().fill(pOut, count);
			};
		}

		void BenchmarkVerify(benchmark::State& state) {
			auto numFailures = 0u;
			std::vector<uint8_t> buffer(Data_Size);
			Signature signature;

			for (auto _ : state) {
				state.PauseTiming();
				auto keyPair = CreateRandomKeyPair();
				bench::FillWithRandomData(buffer);
				crypto::Sign(keyPair, buffer, signature);
				state.ResumeTiming();

				if (!crypto::Verify(keyPair.publicKey(), buffer, signature))
					++numFailures;
			}

			state.SetBytesProcessed(static_cast<int64_t>(Data_Size * state.iterations()));
			if (0 != numFailures)
				CATAPULT_LOG(warning) << numFailures << " calls to Verify failed";
		}

		void BenchmarkVerifyMulti(benchmark::State& state) {
			auto numFailures = 0u;
			constexpr auto Batch_Size = 100;
			std::vector<Signature> signatures(Batch_Size);
			std::vector<std::vector<uint8_t>> buffers(Batch_Size);

			for (auto _ : state) {
				state.PauseTiming();
				std::vector<KeyPair> keyPairs;
				std::vector<SignatureInput> signatureInputs;
				keyPairs.reserve(Batch_Size);
				for (auto i = 0u; i < Batch_Size; ++i) {
					keyPairs.push_back(CreateRandomKeyPair());
					buffers[i].resize(Data_Size);
					bench::FillWithRandomData(buffers[i]);
					crypto::Sign(keyPairs[i], buffers[i], signatures[i]);
					signatureInputs.push_back(SignatureInput({ keyPairs[i].publicKey(), { buffers[i] }, signatures[i] }));
				}

				state.ResumeTiming();

				if (!crypto::VerifyMulti(CreateRandomFiller(), signatureInputs.data(), signatureInputs.size()).second)
					++numFailures;
			}

			state.SetBytesProcessed(static_cast<int64_t>(Data_Size * Batch_Size * state.iterations()));
			if (0 != numFailures)
				CATAPULT_LOG(warning) << numFailures << " calls to VerifyMulti failed";
		}
	}
}}

void RegisterTests();
void RegisterTests() {
	benchmark::RegisterBenchmark("BenchmarkVerify", catapult::crypto::BenchmarkVerify)
			->UseRealTime()
			->Threads(1)
			->Threads(2)
			->Threads(4)
			->Threads(8);

	benchmark::RegisterBenchmark("BenchmarkVerifyMulti", catapult::crypto::BenchmarkVerifyMulti)
			->UseRealTime()
			->Threads(1)
			->Threads(2)
			->Threads(4)
			->Threads(8);
}
