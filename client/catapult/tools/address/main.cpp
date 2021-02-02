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
#include "tools/AccountTool.h"
#include "catapult/model/Address.h"
#include "catapult/utils/ConfigurationValueParsers.h"

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

		// region AddressInspectorTool

		class AddressInspectorTool : public AccountTool {
		public:
			AddressInspectorTool() : AccountTool("Address Inspector Tool", AccountTool::InputDisposition::Required)
			{}

		private:
			void prepareAdditionalOptions(OptionsBuilder& optionsBuilder) override {
				optionsBuilder("mode,m",
						OptionsValue<std::string>()->required(),
						"mode, possible values: encoded, decoded, public, secret");
			}

			void process(const Options& options, const std::vector<std::string>& values, AccountPrinter& printer) override {
				auto mode = ParseMode(options["mode"].as<std::string>());
				for (const auto& value : values) {
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
			}
		};

		// endregion
	}
}}}

int main(int argc, const char** argv) {
	catapult::tools::address::AddressInspectorTool tool;
	return catapult::tools::ToolMain(argc, argv, tool);
}
