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
#include "catapult/extensions/Bip32.h"
#include "catapult/extensions/Bip39.h"
#include "catapult/thread/IoThreadPool.h"
#include "catapult/utils/SpinLock.h"
#include <boost/asio.hpp>
#include <thread>

namespace catapult { namespace tools { namespace addressgen {

	namespace {
		constexpr uint32_t MAX_WALLET_ACCOUNTS = 10;

		// region basic matching

		class GeneratorFacade {
		public:
			GeneratorFacade(model::NetworkIdentifier networkIdentifier, uint32_t numThreads, AccountPrinter& printer)
					: m_networkIdentifier(networkIdentifier)
					, m_numThreads(numThreads)
					, m_printer(printer)
			{}

		public:
			void generate(uint32_t count) const {
				std::atomic<uint32_t> numGenerated(0);
				runGenerator([count, &numGenerated, &printer = m_printer](const auto& mnemonic, auto&& keyPair) {
					if (++numGenerated <= count) {
						printer.print(mnemonic, keyPair);
						return true;
					}

					return false;
				});
			}

		void matchAll(MultiAddressMatcher& matcher, bool showProgress) const {
			runGenerator([&matcher, showProgress, &printer = m_printer](const auto& mnemonic, auto&& keyPair) {
				auto matchResult = matcher.accept(std::move(keyPair));
				if (matchResult.second || (showProgress && matchResult.first))
					printer.print(mnemonic, *matchResult.first);

				return !matcher.isComplete();
			});
		}

		private:
			void runGenerator(const predicate<const std::string&, crypto::KeyPair&&>& acceptKeyPair) const {
				auto coinId = static_cast<uint32_t>(model::NetworkIdentifier::Mainnet == m_networkIdentifier ? 4343 : 1);

				utils::SpinLock acceptLock;
				auto pPool = CreateStartedThreadPool(m_numThreads);
				for (auto i = 0u; i < pPool->numWorkerThreads(); ++i) {
					pPool->ioContext().dispatch([acceptKeyPair, coinId, &acceptLock]() {
						auto randomGenerator = crypto::SecureRandomGenerator();

						for (;;) {
							std::vector<uint8_t> entropy(32);
							randomGenerator.fill(entropy.data(), entropy.size());

							auto mnemonic = extensions::Bip39EntropyToMnemonic(entropy);
							auto seed = extensions::Bip39MnemonicToSeed(mnemonic, "");

							for (uint32_t accountIndex = 0; accountIndex < MAX_WALLET_ACCOUNTS; ++accountIndex) {
								auto bip32Node = extensions::Bip32Node::FromSeed(seed).derive({ 44, coinId, accountIndex, 0, 0 });
								auto keyPair = extensions::Bip32Node::ExtractKeyPair(std::move(bip32Node));

								utils::SpinLockGuard guard(acceptLock);
								if (!acceptKeyPair(mnemonic, std::move(keyPair)))
									return;
							}
						}
					});
				}

				pPool->join();
			}

		private:
			model::NetworkIdentifier m_networkIdentifier;
			uint32_t m_numThreads;
			AccountPrinter& m_printer;
		};

		void AddSearchPatterns(MultiAddressMatcher& matcher, const std::vector<std::string>& values, uint32_t count) {
			for (auto i = 0u; i < count; ++i) {
				for (const auto& value : values)
					matcher.addSearchPattern(value);
			}
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
				optionsBuilder("showProgress",
						OptionsSwitch(),
						"true to print partial matches");
			}

			void process(const Options& options, const std::vector<std::string>& values, AccountPrinter& printer) override {
				// network value is validated before process is called
				model::NetworkIdentifier networkIdentifier;
				model::TryParseValue(options["network"].as<std::string>(), networkIdentifier);

				auto count = options["count"].as<uint32_t>();
				auto numThreads = options["threads"].as<uint32_t>();
				GeneratorFacade generatorFacade(networkIdentifier, numThreads, printer);

				if (values.empty() || values[0].empty()) {
					generatorFacade.generate(count);
				} else {
					MultiAddressMatcher matcher(networkIdentifier);
					AddSearchPatterns(matcher, values, count);
					generatorFacade.matchAll(matcher, options["showProgress"].as<bool>());
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
