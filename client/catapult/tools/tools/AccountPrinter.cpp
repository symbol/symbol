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

#include "AccountPrinter.h"
#include "catapult/model/Address.h"
#include "catapult/utils/ConfigurationValueParsers.h"

namespace catapult { namespace tools {

	namespace {
		[[noreturn]]
		void ThrowInvalidArgument(const char* key, const std::string& value) {
			std::ostringstream out;
			out << "'" << value << "' is not a valid " << key;
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}
	}

	AccountPrinterFormat ParseAccountPrinterFormat(const std::string& str) {
		static const std::array<std::pair<const char*, AccountPrinterFormat>, 2> String_To_Account_Printer_Format_Pairs{{
			{ "pretty", AccountPrinterFormat::Pretty },
			{ "csv", AccountPrinterFormat::Csv }
		}};

		AccountPrinterFormat format;
		if (!utils::TryParseEnumValue(String_To_Account_Printer_Format_Pairs, str, format))
			ThrowInvalidArgument("format", str);

		return format;
	}

	namespace {
		model::NetworkIdentifier ParseNetworkIdentifier(const std::string& str) {
			model::NetworkIdentifier networkIdentifier;
			if (!model::TryParseValue(str, networkIdentifier))
				ThrowInvalidArgument("network", str);

			return networkIdentifier;
		}

		// region AutoLineEnding

		class AutoLineEnding {
		public:
			AutoLineEnding(std::ostream& out, size_t& counter)
					: m_out(out)
					, m_counter(++counter)
			{}

			~AutoLineEnding() {
				if (0 == --m_counter)
					m_out << std::endl;
			}

		private:
			std::ostream& m_out;
			size_t& m_counter;
		};

		// endregion

		// region CsvAccountPrinter

		class CsvAccountPrinter : public AccountPrinter {
		public:
			explicit CsvAccountPrinter(std::ostream& out)
					: m_out(out)
					, m_counter(0)
			{}

		public:
			void setNetwork(const std::string& networkName) override {
				m_networkIdentifier = ParseNetworkIdentifier(networkName);
			}

		public:
			void print(const Address& address) override {
				AutoLineEnding guard(m_out, m_counter);

				m_out
						<< (!model::IsValidAddress(address, m_networkIdentifier) ? "INVALID" : "")
						<< "," << model::AddressToString(address)
						<< "," << address;
			}

			void print(const Key& publicKey) override {
				AutoLineEnding guard(m_out, m_counter);

				print(model::PublicKeyToAddress(publicKey, m_networkIdentifier));
				m_out << "," << publicKey;
			}

			void print(const crypto::KeyPair& keyPair) override {
				AutoLineEnding guard(m_out, m_counter);

				print(keyPair.publicKey());
				m_out << "," << crypto::Ed25519Utils::FormatPrivateKey(keyPair.privateKey());
			}

		private:
			std::ostream& m_out;
			size_t m_counter;

			model::NetworkIdentifier m_networkIdentifier;
		};

		// endregion

		// region PrettyAccountPrinter

		class PrettyAccountPrinter : public AccountPrinter {
		public:
			explicit PrettyAccountPrinter(std::ostream& out)
					: m_out(out)
					, m_counter(0)
			{}

		public:
			void setNetwork(const std::string& networkName) override {
				m_networkName = networkName;
				m_networkIdentifier = ParseNetworkIdentifier(m_networkName);
			}

		public:
			void print(const Address& address) override {
				AutoLineEnding guard(m_out, m_counter);

				std::string qualifier;
				if (!model::IsValidAddress(address, m_networkIdentifier))
					qualifier = "[INVALID] ";

				m_out
						<< std::setw(Label_Width - static_cast<int>(m_networkName.size()) - 3)
								<< "address (" << m_networkName << "): " << qualifier << model::AddressToString(address) << std::endl
						<< std::setw(Label_Width) << "address decoded: " << address << std::endl;
			}

			void print(const Key& publicKey) override {
				AutoLineEnding guard(m_out, m_counter);

				print(model::PublicKeyToAddress(publicKey, m_networkIdentifier));
				m_out << std::setw(Label_Width) << "public key: " << publicKey << std::endl;
			}

			void print(const crypto::KeyPair& keyPair) override {
				AutoLineEnding guard(m_out, m_counter);

				print(keyPair.publicKey());
				m_out
						<< std::setw(Label_Width) << "private key: "
						<< crypto::Ed25519Utils::FormatPrivateKey(keyPair.privateKey()) << std::endl;
			}

		private:
			static constexpr int Label_Width = 24;

		private:
			std::ostream& m_out;
			size_t m_counter;

			std::string m_networkName;
			model::NetworkIdentifier m_networkIdentifier;
		};

		// endregion
	}

	std::unique_ptr<AccountPrinter> CreateAccountPrinter(std::ostream& out, AccountPrinterFormat format) {
		if (AccountPrinterFormat::Csv == format)
			return std::make_unique<CsvAccountPrinter>(out);

		return std::make_unique<PrettyAccountPrinter>(out);
	}

	namespace {
		// region AggregateAccountPrinter

		class AggregateAccountPrinter : public AccountPrinter {
		public:
			explicit AggregateAccountPrinter(std::vector<std::unique_ptr<AccountPrinter>>&& printers) : m_printers(std::move(printers))
			{}

		public:
			void setNetwork(const std::string& networkName) override {
				for (auto& pPrinter : m_printers)
					pPrinter->setNetwork(networkName);
			}

		public:
			void print(const Address& address) override {
				for (auto& pPrinter : m_printers)
					pPrinter->print(address);
			}

			void print(const Key& publicKey) override {
				for (auto& pPrinter : m_printers)
					pPrinter->print(publicKey);
			}

			void print(const crypto::KeyPair& keyPair) override {
				for (auto& pPrinter : m_printers)
					pPrinter->print(keyPair);
			}

		private:
			std::vector<std::unique_ptr<AccountPrinter>> m_printers;
		};

		// endregion
	}

	std::unique_ptr<AccountPrinter> CreateAggregateAccountPrinter(std::vector<std::unique_ptr<AccountPrinter>>&& printers) {
		return std::make_unique<AggregateAccountPrinter>(std::move(printers));
	}
}}
