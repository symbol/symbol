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

#include "tools/ToolMain.h"
#include "MultiAddressMatcher.h"
#include "tools/AccountTool.h"
#include "tools/ToolThreadUtils.h"
#include "catapult/crypto/SecureRandomGenerator.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/SpinLock.h"
#include <boost/asio.hpp>
#include <thread>

namespace catapult { namespace tools { namespace addressgen {

	namespace {
		// region basic matching

		void RunGenerator(uint32_t numThreads, const predicate<crypto::PrivateKey&&>& acceptPrivateKey) {
			utils::SpinLock acceptLock;
			auto pPool = CreateStartedThreadPool(numThreads);
			for (auto i = 0u; i < pPool->numWorkerThreads(); ++i) {
				pPool->ioContext().dispatch([acceptPrivateKey, &acceptLock]() {
					auto randomGenerator = crypto::SecureRandomGenerator();

					for (;;) {
						Key privateKeyBuffer;
						randomGenerator.fill(privateKeyBuffer.data(), privateKeyBuffer.size());
						auto privateKey = crypto::PrivateKey::FromBufferSecure(privateKeyBuffer);

						utils::SpinLockGuard guard(acceptLock);
						if (!acceptPrivateKey(std::move(privateKey)))
							return;
					}
				});
			}

			pPool->join();
		}

		void Generate(uint32_t count, uint32_t numThreads, AccountPrinter& printer) {
			std::atomic<uint32_t> numGenerated(0);
			RunGenerator(numThreads, [count, &printer, &numGenerated](auto&& privateKey) {
				auto keyPair = crypto::KeyPair::FromPrivate(std::move(privateKey));

				if (++numGenerated <= count) {
					printer.print(keyPair);
					return true;
				}

				return false;
			});
		}

		void AddSearchPatterns(MultiAddressMatcher& matcher, const std::vector<std::string>& values, uint32_t count) {
			for (auto i = 0u; i < count; ++i) {
				for (const auto& value : values)
					matcher.addSearchPattern(value);
			}
		}

		void MatchAll(MultiAddressMatcher& matcher, uint32_t numThreads, AccountPrinter& printer) {
			RunGenerator(numThreads, [&matcher, &printer](auto&& privateKey) {
				const auto* pNewKeyPair = matcher.accept(std::move(privateKey));
				if (pNewKeyPair)
					printer.print(*pNewKeyPair);

				return !matcher.isComplete();
			});
		}

		// endregion

		// region AddressGeneratorTool

		class AddressGeneratorTool : public AccountTool {
		public:
			AddressGeneratorTool() : AccountTool("Address Generator Tool", AccountTool::InputDisposition::Optional)
			{}

		private:
			void prepareAdditionalOptions(OptionsBuilder& optionsBuilder) override {
				optionsBuilder("count,c",
						OptionsValue<uint32_t>()->default_value(1),
						"number of random keys to generate for each pattern");
				optionsBuilder("threads,t",
						OptionsValue<uint32_t>()->default_value(2 * std::thread::hardware_concurrency()),
						"number of threads to use");
			}

			void process(const Options& options, const std::vector<std::string>& values, AccountPrinter& printer) override {
				// network value is validated before process is called
				model::NetworkIdentifier networkIdentifier;
				model::TryParseValue(options["network"].as<std::string>(), networkIdentifier);

				auto count = options["count"].as<uint32_t>();
				auto numThreads = options["threads"].as<uint32_t>();
				if (values.empty() || values[0].empty()) {
					Generate(count, numThreads, printer);
				} else {
					MultiAddressMatcher matcher(networkIdentifier);
					AddSearchPatterns(matcher, values, count);
					MatchAll(matcher, numThreads, printer);
				}
			}
		};

		// endregion
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::addressgen::AddressGeneratorTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
