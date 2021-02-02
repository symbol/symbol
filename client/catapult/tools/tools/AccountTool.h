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

#pragma once
#include "AccountPrinter.h"
#include <filesystem>
#include <fstream>

namespace catapult { namespace tools {

	/// Base class for a tool that performs operations on one or more account-related inputs.
	class AccountTool : public Tool {
	protected:
		/// Disposition of input option.
		enum class InputDisposition {
			/// Required.
			Required,

			/// Optional (default is empty string).
			Optional
		};

	public:
		/// Creates a tool with \a name and input disposition (\a inputDisposition).
		AccountTool(const std::string& name, InputDisposition inputDisposition)
				: m_name(name)
				, m_inputDisposition(inputDisposition)
		{}

	public:
		std::string name() const override final {
			return m_name;
		}

		void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional&) override final {
			optionsBuilder("network,n",
					OptionsValue<std::string>()->default_value("private"),
					"network, possible values: private (default), private-test, public, public-test");

			optionsBuilder("input,i",
					InputDisposition::Required == m_inputDisposition
							? OptionsValue<std::string>()->required()
							: OptionsValue<std::string>()->default_value(""),
					"input value (comma-delimited) or file");
			optionsBuilder("output,o",
					OptionsValue<std::string>(),
					"(optional) output file");
			optionsBuilder("format,f",
					OptionsValue<std::string>()->default_value("pretty"),
					"output format, possible values: pretty (default), csv");

			optionsBuilder("suppressConsole",
					OptionsSwitch(),
					"true to suppress console output");

			prepareAdditionalOptions(optionsBuilder);
		}

		int run(const Options& options) override final {
			auto input = options["input"].as<std::string>();
			auto format = ParseAccountPrinterFormat(options["format"].as<std::string>());
			auto networkName = options["network"].as<std::string>();

			std::vector<std::unique_ptr<AccountPrinter>> printers;
			if (!options["suppressConsole"].as<bool>())
				printers.push_back(CreateAccountPrinter(std::cout, format));

			std::unique_ptr<std::ofstream> pFout;
			if (options.count("output")) {
				pFout = std::make_unique<std::ofstream>(options["output"].as<std::string>(), std::ios::out);
				printers.push_back(CreateAccountPrinter(*pFout, format));
			}

			auto pAggregatePrinter = CreateAggregateAccountPrinter(std::move(printers));
			pAggregatePrinter->setNetwork(networkName);
			processAll(options, input, *pAggregatePrinter);
			return 0;
		}

	private:
		void processAll(const Options& options, const std::string& input, AccountPrinter& printer) {
			std::vector<std::string> values;

			std::string value;
			if (std::filesystem::exists(input)) {
				std::ifstream fin(input, std::ios::in);

				while (fin >> value)
					values.push_back(value);
			} else {
				std::istringstream stream(input);
				while (stream.good()) {
					std::getline(stream, value, ',');
					values.push_back(value);
				}
			}

			process(options, values, printer);
		}

	private:
		/// Prepare additional named (\a optionsBuilder) options of the tool.
		virtual void prepareAdditionalOptions(OptionsBuilder& optionsBuilder) = 0;

		/// Processes \a values given \a options and \a printer.
		virtual void process(const Options& options, const std::vector<std::string>& values, AccountPrinter& printer) = 0;

	private:
		std::string m_name;
		InputDisposition m_inputDisposition;
	};
}}
