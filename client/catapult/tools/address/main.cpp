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
#include "tools/AccountPrinter.h"
#include "catapult/model/Address.h"
#include "catapult/utils/ConfigurationValueParsers.h"
#include <filesystem>
#include <fstream>

namespace catapult { namespace tools { namespace address {

	namespace {
		// region Mode

		enum class Mode { Encoded_Address, Decoded_Address, Public_Key, Secret_Key };

		Mode ParseMode(const std::string& str) {
			static const std::array<std::pair<const char*, Mode>, 4> String_To_Mode_Pairs{{
				{ "encoded", Mode::Encoded_Address },
				{ "decoded", Mode::Decoded_Address },
				{ "public", Mode::Public_Key },
				{ "secret", Mode::Secret_Key }
			}};

			Mode mode;
			if (!utils::TryParseEnumValue(String_To_Mode_Pairs, str, mode)) {
				std::ostringstream out;
				out << "'" << str << "' is not a valid mode";
				CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
			}

			return mode;
		}

		// endregion

		// region AddressTool

		class AddressTool : public Tool {
		public:
			std::string name() const override {
				return "Address Tool";
			}

			void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override {
				optionsBuilder("network,n",
						OptionsValue<std::string>()->default_value("private"),
						"network, possible values: private (default), private-test, public, public-test");

				optionsBuilder("mode,m",
						OptionsValue<std::string>()->required(),
						"mode, possible values: encoded, decoded, public, secret");

				optionsBuilder("input,i",
						OptionsValue<std::string>()->required(),
						"input value or file");

				optionsBuilder("output,o",
						OptionsValue<std::string>(),
						"(optional) output file");

				optionsBuilder("format,f",
						OptionsValue<std::string>()->default_value("pretty"),
						"output format, possible values: pretty (default), csv");

				optionsBuilder("suppressConsole",
						OptionsSwitch(),
						"true to suppress console output");
			}

			int run(const Options& options) override {
				auto mode = ParseMode(options["mode"].as<std::string>());
				auto input = options["input"].as<std::string>();
				auto format = ParseAccountPrinterFormat(options["format"].as<std::string>());
				auto networkName = options["network"].as<std::string>();

				if (!options["suppressConsole"].as<bool>()) {
					auto pPrinter = CreatePrinter(std::cout, format, networkName);
					ProcessAll(mode, input, *pPrinter);
				}

				if (options.count("output")) {
					std::ofstream fout(options["output"].as<std::string>(), std::ios::out);
					auto pPrinter = CreatePrinter(fout, format, networkName);
					ProcessAll(mode, input, *pPrinter);
				}

				return 0;
			}

		private:
			static std::unique_ptr<AccountPrinter> CreatePrinter(
					std::ostream& out,
					AccountPrinterFormat format,
					const std::string& networkName) {
				auto pPrinter = CreateAccountPrinter(out, format);
				pPrinter->setNetwork(networkName);
				return pPrinter;
			}

			static void ProcessAll(Mode mode, const std::string& input, AccountPrinter& printer) {
				if (std::filesystem::exists(input)) {
					std::ifstream fin(input, std::ios::in);

					std::string line;
					while (fin >> line)
						Process(mode, line, printer);
				} else {
					Process(mode, input, printer);
				}
			}

			static void Process(Mode mode, const std::string& value, AccountPrinter& printer) {
				switch (mode) {
				case Mode::Encoded_Address:
					printer.print(model::StringToAddress(value));
					break;

				case Mode::Decoded_Address:
					printer.print(utils::ParseByteArray<Address>(value));
					break;

				case Mode::Public_Key:
					printer.print(utils::ParseByteArray<Key>(value));
					break;

				case Mode::Secret_Key:
					printer.print(crypto::KeyPair::FromString(value));
					break;
				}
			}
		};

		// endregion
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::address::AddressTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
