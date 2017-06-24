#include "tools/ToolMain.h"
#include "tools/ToolKeys.h"
#include "tools/ToolUtils.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/config/LocalNodeConfiguration.h"
#include "catapult/extensions/RemoteDiagnosticApi.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/DiagnosticCounterId.h"
#include <boost/filesystem/path.hpp>
#include <thread>

namespace catapult { namespace tools { namespace uts {

	namespace {
		config::LocalNodeConfiguration LoadConfiguration(const std::string& resourcesPathStr) {
			boost::filesystem::path resourcesPath = resourcesPathStr;
			resourcesPath /= "resources";
			std::cout << "loading resources from " << resourcesPath << std::endl;
			return config::LocalNodeConfiguration::LoadFromPath(resourcesPath);
		}

		struct NodeInfo {
		public:
			explicit NodeInfo(const ionet::Node& node) : Node(node)
			{}

		public:
			std::shared_ptr<ionet::PacketIo> pIo;
			ionet::Node Node;
			Height ChainHeight;
			model::ChainScore ChainScore;
			model::EntityRange<model::DiagnosticCounterValue> DiagnosticCounters;
		};

		using NodeInfoPointer = std::shared_ptr<NodeInfo>;
		using NodeInfoFuture = thread::future<NodeInfoPointer>;

		thread::future<bool> CreateChainInfoFuture(const std::shared_ptr<ionet::PacketIo>& pIo, NodeInfo& nodeInfo) {
			auto pApi = api::CreateRemoteChainApi(pIo);
			return pApi->chainInfo().then([&nodeInfo](auto&& infoFuture) {
				try {
					auto info = infoFuture.get();
					nodeInfo.ChainHeight = info.Height;
					nodeInfo.ChainScore = info.Score;
					return true;
				} catch (...) {
					// suppress
					return false;
				}
			});
		}

		thread::future<bool> CreateDiagnosticCountersFuture(const std::shared_ptr<ionet::PacketIo>& pIo, NodeInfo& nodeInfo) {
			auto pApi = extensions::CreateRemoteDiagnosticApi(pIo);
			return pApi->diagnosticCounters().then([&nodeInfo](auto&& countersFuture) {
				try {
					nodeInfo.DiagnosticCounters = countersFuture.get();
					return true;
				} catch (...) {
					// suppress
					return false;
				}
			});
		}

		NodeInfoFuture CreateNodeInfoFuture(
				const crypto::KeyPair& clientKeyPair,
				const ionet::Node& node,
				const std::shared_ptr<thread::IoServiceThreadPool>& pPool) {
			auto pNodeInfo = std::make_shared<NodeInfo>(node);
			return thread::compose(
					ConnectToNode(clientKeyPair, node, pPool),
					[pNodeInfo](auto&& ioFuture) {
						try {
							auto pIo = ioFuture.get();
							std::vector<thread::future<bool>> infoFutures;
							infoFutures.emplace_back(CreateChainInfoFuture(pIo, *pNodeInfo));
							infoFutures.emplace_back(CreateDiagnosticCountersFuture(pIo, *pNodeInfo));
							return thread::when_all(std::move(infoFutures)).then([pNodeInfo](auto&&) { return pNodeInfo; });
						} catch (...) {
							// suppress
							CATAPULT_LOG(error) << pNodeInfo->Node << " appears to be offline";
							return thread::make_ready_future(decltype(pNodeInfo)(pNodeInfo));
						}
					});
		}

		template<typename T>
		size_t GetStringSize(const T& value) {
			std::ostringstream out;
			out << value;
			return out.str().size();
		}

		std::string FormatCounterValue(uint64_t value) {
			auto str = std::to_string(value);
			auto iter = str.end();
			constexpr auto Num_Grouping_Digits = 3;
			while (std::distance(str.begin(), iter) > Num_Grouping_Digits) {
				iter -= Num_Grouping_Digits;
				str.insert(iter, '\'');
			}

			return str;
		}

		utils::LogLevel MapRelativeHeightToLogLevel(Height height, Height maxChainHeight) {
			return Height() == height
					? utils::LogLevel::Error
					: maxChainHeight > height ? utils::LogLevel::Warning : utils::LogLevel::Info;
		}

		size_t GetLevelLeftPadding(utils::LogLevel level) {
			// add left padding in order to align all level names with longest level name (warning)
			switch (level) {
			case utils::LogLevel::Error:
			case utils::LogLevel::Debug:
			case utils::LogLevel::Trace:
			case utils::LogLevel::Fatal:
				return 2;

			case utils::LogLevel::Info:
				return 3;

			default:
				return 0;
			}
		}

		void PrettyPrintSummary(const std::vector<NodeInfoPointer>& nodeInfos) {
			Height maxChainHeight;
			size_t maxNodeNameSize = 0;
			size_t maxHeightSize = 0;
			for (const auto& pNodeInfo : nodeInfos) {
				maxChainHeight = std::max(maxChainHeight, pNodeInfo->ChainHeight);
				maxNodeNameSize = std::max(maxNodeNameSize, GetStringSize(pNodeInfo->Node));
				maxHeightSize = std::max(maxHeightSize, GetStringSize(pNodeInfo->ChainHeight));
			}

			for (const auto& pNodeInfo : nodeInfos) {
				auto level = MapRelativeHeightToLogLevel(pNodeInfo->ChainHeight, maxChainHeight);
				CATAPULT_LOG_LEVEL(level)
						<< std::string(GetLevelLeftPadding(level), ' ') << std::setw(static_cast<int>(maxNodeNameSize)) << pNodeInfo->Node
						<< " at height " << std::setw(static_cast<int>(maxHeightSize)) << pNodeInfo->ChainHeight
						<< " with score " << pNodeInfo->ChainScore;
			}
		}

		void PrettyPrintCounters(const NodeInfo& nodeInfo) {
			std::ostringstream table;
			table << nodeInfo.Node << std::endl;

			// insert (name, formatted-value) pairs into a map for sorting by name
			size_t maxValueSize = 0;
			std::map<std::string, std::string> sortedCounters;
			for (const auto& counterValue : nodeInfo.DiagnosticCounters) {
				auto value = FormatCounterValue(counterValue.Value);
				sortedCounters.emplace(utils::DiagnosticCounterId(counterValue.Id).name(), value);
				maxValueSize = std::max(maxValueSize, value.size());
			}

			constexpr auto Num_Counters_Per_Line = 4;
			auto numCountersPrinted = 0;
			for (const auto& pair : sortedCounters) {
				table
						<< std::setw(utils::DiagnosticCounterId::Max_Counter_Name_Size) << std::right << pair.first << " : "
						<< std::setw(static_cast<int>(maxValueSize)) << std::left << pair.second;

				table << " | ";
				if (0 == ++numCountersPrinted % Num_Counters_Per_Line)
					table << std::endl;
			}

			CATAPULT_LOG(debug) << table.str();
		}

		void PrettyPrint(const std::vector<NodeInfoPointer>& nodeInfos) {
			CATAPULT_LOG(info) << "--- COUNTERS for known peers ---";
			for (const auto& pNodeInfo : nodeInfos)
				PrettyPrintCounters(*pNodeInfo);

			CATAPULT_LOG(info) << "--- SUMMARY for known peers ---";
			PrettyPrintSummary(nodeInfos);
		}

		class HealthTool : public Tool {
		public:
			std::string name() const override {
				return "Catapult Block Chain Health Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional& positional) override {
				optionsBuilder("resources,r",
						OptionsValue<std::string>(m_resourcesPath)->default_value(".."),
						"the path to the resources directory");
				positional.add("resources", -1);
			}

			int run(const Options&) override {
				auto config = LoadConfiguration(m_resourcesPath);

				auto clientKeyPair = GenerateRandomKeyPair();
				auto pPool = CreateStartedThreadPool();

				std::vector<NodeInfoFuture> nodeInfoFutures;
				for (const auto& node : config.Peers) {
					CATAPULT_LOG(debug) << "preparing to get stats from node " << node;
					nodeInfoFutures.push_back(CreateNodeInfoFuture(clientKeyPair, node, pPool));
				}

				auto finalFuture = thread::when_all(std::move(nodeInfoFutures)).then([](auto&& allFutures) {
					std::vector<NodeInfoPointer> nodeInfos;
					for (auto& nodeInfoFuture : allFutures.get())
						nodeInfos.push_back(nodeInfoFuture.get());

					PrettyPrint(nodeInfos);
				});

				finalFuture.get();
				return 0;
			}

		private:
			std::string m_resourcesPath;
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::uts::HealthTool healthTool;
	return catapult::tools::ToolMain(argc, argv, healthTool);
}
