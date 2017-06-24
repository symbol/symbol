#include "tools/ToolMain.h"
#include "tools/ToolKeys.h"
#include "tools/ToolUtils.h"
#include "catapult/crypto/Signer.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "catapult/thread/ParallelFor.h"
#include "catapult/utils/StackLogger.h"

namespace catapult { namespace tools { namespace benchmark {

	namespace {
		struct BenchmarkEntry {
			std::vector<uint8_t> Data;
			catapult::Signature Signature;
			bool IsVerified = false;
		};

		class BenchmarkTool : public Tool {
		public:
			std::string name() const override {
				return "Benchmark Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				optionsBuilder("num threads,t",
						OptionsValue<uint32_t>(m_numThreads)->default_value(0),
						"the number of threads");
				optionsBuilder("num partitions,p",
						OptionsValue<uint32_t>(m_numPartitions)->default_value(0),
						"the number of partitions");
				optionsBuilder("ops / partition,o",
						OptionsValue<uint32_t>(m_opsPerPartition)->default_value(1000),
						"the number of operations per partition");
				optionsBuilder("data size,s",
						OptionsValue<uint32_t>(m_dataSize)->default_value(148),
						"the size of the data to generate");
			}

			int run(const Options&) override {
				m_numThreads = 0 != m_numThreads ? m_numThreads : std::thread::hardware_concurrency();
				m_numPartitions = 0 != m_numPartitions ? m_numPartitions : m_numThreads;

				CATAPULT_LOG(info)
						<< "num threads (" << m_numThreads
						<< "), num partitions (" << m_numPartitions
						<< "), ops / partition (" << m_opsPerPartition
						<< "), data size (" << m_dataSize << ")";

				auto keyPair = GenerateRandomKeyPair();
				auto entries = std::vector<BenchmarkEntry>(m_numPartitions * m_opsPerPartition);
				auto pPool = CreateStartedThreadPool(m_numThreads);

				CATAPULT_LOG(info) << "num operations (" << entries.size() << ")";

				RunParallel("Data Generation", *pPool, entries, [dataSize = m_dataSize](auto& entry) {
					entry.Data.resize(dataSize);
					std::generate_n(entry.Data.begin(), entry.Data.size(), std::rand);
				});

				RunParallel("Signature", *pPool, entries, [&keyPair](auto& entry) {
					crypto::Sign(keyPair, entry.Data, entry.Signature);
				});

				RunParallel("Verify", *pPool, entries, [&keyPair](auto& entry) {
					entry.IsVerified = crypto::Verify(keyPair.publicKey(), entry.Data, entry.Signature);
					if (!entry.IsVerified)
						CATAPULT_LOG(warning) << "could not verify data!";
				});

				return 0;
			}

		private:
			template<typename TAction>
			uint64_t RunParallel(
					const char* testName,
					thread::IoServiceThreadPool& pool,
					std::vector<BenchmarkEntry>& entries,
					TAction action) const {
				utils::StackLogger stopwatch(testName, utils::LogLevel::Info);
				thread::ParallelFor(pool.service(), entries, m_numPartitions, [action](auto& entry) {
					action(entry);
					return true;
				}).get();

				auto elapsedMillis = stopwatch.millis();
				auto elapsedMicrosPerOp = elapsedMillis * 1000u / entries.size();
				auto opsPerSecond = 0 == elapsedMillis ? 0 : entries.size() * 1000u / elapsedMillis;
				CATAPULT_LOG(info)
						<< (0 == opsPerSecond ? "???" : std::to_string(opsPerSecond)) << " ops/s "
						<< "(elapsed time " << elapsedMillis << "ms, " << elapsedMicrosPerOp << "us/op)";
				return elapsedMillis;
			}

		private:
			uint32_t m_numThreads;
			uint32_t m_numPartitions;
			uint32_t m_opsPerPartition;
			uint32_t m_dataSize;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::benchmark::BenchmarkTool benchmarkTool;
	return catapult::tools::ToolMain(argc, argv, benchmarkTool);
}
