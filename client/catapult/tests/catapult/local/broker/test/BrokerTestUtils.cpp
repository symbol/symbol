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

#include "BrokerTestUtils.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/io/Stream.h"
#include "catapult/io/TransactionInfoSerializer.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/TransactionStatusTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"

namespace catapult { namespace test {

	namespace {
		void WriteTransactionInfos(io::OutputStream& outputStream, uint8_t operationType) {
			auto transactionInfos = CopyTransactionInfosToSet(CreateTransactionInfosWithOptionalAddresses(3));
			io::Write8(outputStream, operationType);
			io::WriteTransactionInfos(transactionInfos, outputStream);
		}

		void WriteCosignature(io::OutputStream& outputStream) {
			auto cosignature = CreateRandomDetachedCosignature();
			auto transactionInfo = CreateRandomTransactionInfo();
			transactionInfo.OptionalExtractedAddresses = GenerateRandomUnresolvedAddressSetPointer(Random() % 2 + 1);

			io::Write8(outputStream, utils::to_underlying_type(subscribers::PtChangeOperationType::Add_Cosignature));
			outputStream.write({ reinterpret_cast<const uint8_t*>(&cosignature), sizeof(model::Cosignature) });
			io::WriteTransactionInfo(transactionInfo, outputStream);
		}
	}

	void WriteRandomUtChange(io::OutputStream& outputStream) {
		auto operationType = 0 == RandomByte() % 2
				? subscribers::UtChangeOperationType::Add
				: subscribers::UtChangeOperationType::Remove;
		WriteTransactionInfos(outputStream, utils::to_underlying_type(operationType));
	}

	void WriteRandomPtChange(io::OutputStream& outputStream) {
		auto value = RandomByte();
		if (value > 127) {
			WriteCosignature(outputStream);
			return;
		}

		auto operationType = 0 == value % 2
				? subscribers::PtChangeOperationType::Add_Partials
				: subscribers::PtChangeOperationType::Remove_Partials;
		WriteTransactionInfos(outputStream, utils::to_underlying_type(operationType));
	}

	void WriteRandomTransactionStatus(io::OutputStream& outputStream) {
		auto notification = GenerateRandomTransactionStatusNotification(141);
		WriteTransactionStatusNotification(outputStream, notification);
	}
}}
