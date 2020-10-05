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

#include "ApiNodeHealthUtils.h"
#include "tools/NetworkCensusTool.h"
#include "tools/ToolMain.h"
#include "tools/ToolThreadUtils.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/extensions/RemoteDiagnosticApi.h"
#include "catapult/utils/DiagnosticCounterId.h"
#include "catapult/utils/Functional.h"

namespace catapult { namespace tools { namespace health {

	namespace {
		// region node info

		struct NodeInfo {
		public:
			explicit NodeInfo(const ionet::Node& node) : Node(node)
			{}

		public:
			ionet::Node Node;
			Height ChainHeight;
			Height FinalizedChainHeight;
			model::ChainScore ChainScore;
			model::EntityRange<model::DiagnosticCounterValue> DiagnosticCounters;
		};

		using NodeInfoPointer = NetworkCensusTool<NodeInfo>::NodeInfoPointer;

		// endregion

		// region futures

		thread::future<api::ChainStatistics> StartChainStatisticsFuture(
				thread::IoThreadPool& pool,
				ionet::PacketIo& io,
				NodeInfo& nodeInfo) {
			if (!HasFlag(ionet::NodeRoles::Peer, nodeInfo.Node.metadata().Roles)) {
				return CreateApiNodeChainStatisticsFuture(pool, nodeInfo.Node);
			} else {
				auto pApi = api::CreateRemoteChainApiWithoutRegistry(io);
				return pApi->chainStatistics();
			}
		}

		thread::future<bool> CreateChainStatisticsFuture(thread::IoThreadPool& pool, ionet::PacketIo& io, NodeInfo& nodeInfo) {
			return StartChainStatisticsFuture(pool, io, nodeInfo).then([&nodeInfo](auto&& chainStatisticsFuture) {
				return UnwrapFutureAndSuppressErrors("querying chain statistics", std::move(chainStatisticsFuture), [&nodeInfo](
						const auto& chainStatistics) {
					nodeInfo.ChainHeight = chainStatistics.Height;
					nodeInfo.FinalizedChainHeight = chainStatistics.FinalizedHeight;
					nodeInfo.ChainScore = chainStatistics.Score;
				});
			});
		}

		thread::future<bool> CreateDiagnosticCountersFuture(ionet::PacketIo& io, NodeInfo& nodeInfo) {
			auto pApi = extensions::CreateRemoteDiagnosticApi(io);
			return pApi->diagnosticCounters().then([&nodeInfo](auto&& countersFuture) {
				UnwrapFutureAndSuppressErrors("querying diagnostic counters", std::move(countersFuture), [&nodeInfo](auto&& counters) {
					nodeInfo.DiagnosticCounters = std::move(counters);
				});
			});
		}

		// endregion

		// region formatting

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
					? utils::LogLevel::error
					: maxChainHeight > height ? utils::LogLevel::warning : utils::LogLevel::info;
		}

		size_t GetLevelLeftPadding(utils::LogLevel level) {
			// add left padding in order to align all level names with longest level name (warning)
			switch (level) {
			case utils::LogLevel::error:
			case utils::LogLevel::debug:
			case utils::LogLevel::trace:
			case utils::LogLevel::fatal:
				return 2;

			case utils::LogLevel::info:
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
						<< " [" << (HasFlag(ionet::NodeRoles::Api, pNodeInfo->Node.metadata().Roles) ? "API" : "P2P") << "]"
						<< " at height " << std::setw(static_cast<int>(maxHeightSize)) << pNodeInfo->ChainHeight
						<< " (" << pNodeInfo->FinalizedChainHeight << " finalized)"
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

			CATAPULT_LOG(info) << table.str();
		}

		void PrettyPrint(const std::vector<NodeInfoPointer>& nodeInfos) {
			CATAPULT_LOG(info) << "--- COUNTERS for known peers ---";
			for (const auto& pNodeInfo : nodeInfos)
				PrettyPrintCounters(*pNodeInfo);

			CATAPULT_LOG(info) << "--- SUMMARY for known peers ---";
			PrettyPrintSummary(nodeInfos);
		}

		// endregion

		class HealthTool : public NetworkCensusTool<NodeInfo> {
		public:
			HealthTool() : NetworkCensusTool("Health")
			{}

		private:
			std::vector<thread::future<bool>> getNodeInfoFutures(
					thread::IoThreadPool& pool,
					ionet::PacketIo& io,
					const model::NodeIdentity&,
					NodeInfo& nodeInfo) override {
				std::vector<thread::future<bool>> infoFutures;
				infoFutures.emplace_back(CreateChainStatisticsFuture(pool, io, nodeInfo));
				infoFutures.emplace_back(CreateDiagnosticCountersFuture(io, nodeInfo));
				return infoFutures;
			}

			size_t processNodeInfos(const std::vector<NodeInfoPointer>& nodeInfos) override {
				PrettyPrint(nodeInfos);

				return utils::Sum(nodeInfos, [](const auto& pNodeInfo) {
					return Height() == pNodeInfo->ChainHeight ? 1u : 0;
				});
			}
		};
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::health::HealthTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
