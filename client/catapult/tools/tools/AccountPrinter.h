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
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/NetworkIdentifier.h"

namespace catapult { namespace tools {

	/// Printer that prints account identifiers.
	class AccountPrinter {
	public:
		virtual ~AccountPrinter() = default;

	public:
		/// Sets the current network to the network with friendly name \a networkName.
		virtual void setNetwork(const std::string& networkName) = 0;

	public:
		/// Prints \a address.
		virtual void print(const Address& address) = 0;

		/// Prints \a publicKey.
		virtual void print(const Key& publicKey) = 0;

		/// Prints \a keyPair.
		virtual void print(const crypto::KeyPair& keyPair) = 0;
	};

	/// Supported account printer formats.
	enum class AccountPrinterFormat {
		/// Pretty format.
		Pretty,

		/// CSV format.
		Csv
	};

	/// Parses \a str into an account printer format.
	AccountPrinterFormat ParseAccountPrinterFormat(const std::string& str);

	/// Creates an account printer with \a format around \a out.
	std::unique_ptr<AccountPrinter> CreateAccountPrinter(std::ostream& out, AccountPrinterFormat format);

	/// Creates an aggregate account printer around \a printers.
	std::unique_ptr<AccountPrinter> CreateAggregateAccountPrinter(std::vector<std::unique_ptr<AccountPrinter>>&& printers);
}}
