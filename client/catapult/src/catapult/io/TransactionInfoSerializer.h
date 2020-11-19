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
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/EntityInfo.h"

namespace catapult {
	namespace io {
		class InputStream;
		class OutputStream;
	}
}

namespace catapult { namespace io {

	/// Writes \a transactionInfo into \a outputStream.
	void WriteTransactionInfo(const model::TransactionInfo& transactionInfo, OutputStream& outputStream);

	/// Reads transaction info from \a inputStream into \a transactionInfo.
	void ReadTransactionInfo(InputStream& inputStream, model::TransactionInfo& transactionInfo);

	/// Writes \a transactionInfos into \a outputStream.
	void WriteTransactionInfos(const model::TransactionInfosSet& transactionInfos, OutputStream& outputStream);

	/// Reads transaction infos from \a inputStream into \a transactionInfos.
	void ReadTransactionInfos(InputStream& inputStream, model::TransactionInfosSet& transactionInfos);
}}
