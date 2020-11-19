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

#include "EntityDump.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "plugins/txes/transfer/src/model/TransferTransaction.h"
#include "catapult/model/Address.h"
#include "catapult/model/Block.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/Logging.h"
#include <iomanip>

using namespace catapult::model;
using namespace catapult::crypto;

namespace catapult { namespace test {

#define VALANDHEX(x) std::setw(5 * sizeof(x) / 2 + 1) << x << " (" << utils::HexFormat(x) << ")"

	namespace {
		void EntityDump(const TransferTransaction& tx) {
			CATAPULT_LOG(debug) << "  Recipient: " << AddressToString(extensions::CopyToAddress(tx.RecipientAddress));
			CATAPULT_LOG(debug) << "    Message: " << "size:" << VALANDHEX(tx.MessageSize);
			if (tx.MessageSize)
				CATAPULT_LOG(debug) << "  Message D: " << utils::HexFormat(tx.MessagePtr(), tx.MessagePtr() + tx.MessageSize);

			CATAPULT_LOG(debug) << "Mosaics: " << VALANDHEX(tx.MosaicsCount);
			auto pMosaic = tx.MosaicsPtr();
			for (auto i = 0u; i < tx.MosaicsCount; ++i) {
				CATAPULT_LOG(debug) << ":    Id: " << VALANDHEX(pMosaic->MosaicId);
				CATAPULT_LOG(debug) << ":Amount: " << VALANDHEX(pMosaic->Amount);
				pMosaic++;
			}
		}
	}

	void EntityDump(const Transaction& tx) {
		CATAPULT_LOG(debug) << "       Size: " << VALANDHEX(tx.Size);
		CATAPULT_LOG(debug) << "    Version: " << VALANDHEX(tx.Version);
		CATAPULT_LOG(debug) << "       Type: " << VALANDHEX(tx.Type);
		CATAPULT_LOG(debug) << "     Signer: " << tx.SignerPublicKey;
		CATAPULT_LOG(debug) << "  Signature:\n" << tx.Signature;
		CATAPULT_LOG(debug) << "    Max Fee: " << VALANDHEX(tx.MaxFee);
		CATAPULT_LOG(debug) << "   Deadline: " << VALANDHEX(tx.Deadline);

		if (Entity_Type_Transfer == tx.Type)
			EntityDump(static_cast<const TransferTransaction&>(tx));
	}

	void EntityDump(const Block& block) {
		CATAPULT_LOG(debug) << "        Size: " << VALANDHEX(block.Size);
		CATAPULT_LOG(debug) << "     Version: " << VALANDHEX(block.Version);
		CATAPULT_LOG(debug) << "        Type: " << VALANDHEX(block.Type);
		CATAPULT_LOG(debug) << "      Signer: " << block.SignerPublicKey;
		CATAPULT_LOG(debug) << "   Timestamp: " << VALANDHEX(block.Timestamp);
		CATAPULT_LOG(debug) << "   Signature:\n" << block.Signature;
		CATAPULT_LOG(debug) << "      Height: " << VALANDHEX(block.Height);
		CATAPULT_LOG(debug) << "  Difficulty: " << VALANDHEX(block.Difficulty);
		CATAPULT_LOG(debug) << " PrevBlockHa: " << block.PreviousBlockHash;
	}

#undef VALANDHEX
}}
