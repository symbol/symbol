#include "EntityDump.h"
#include "plugins/txes/transfer/src/model/TransferTransaction.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/model/Address.h"
#include "catapult/model/Block.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/utils/Logging.h"
#include <iomanip>

using namespace catapult::model;
using namespace catapult::crypto;
using catapult::utils::HexFormat;

namespace catapult { namespace test {

#define VALANDHEX(x) std::setw(5 * sizeof(x) / 2 + 1) << x << " (" << HexFormat(x) << ")"

	namespace {
		void EntityDump(const TransferTransaction& tx) {
			CATAPULT_LOG(debug) << "  Recipient: " << AddressToString(tx.Recipient);
			CATAPULT_LOG(debug) << "    Message: " << "size:" << VALANDHEX(tx.MessageSize);
			if (tx.MessageSize)
				CATAPULT_LOG(debug) << "  Message D: " << HexFormat(tx.MessagePtr(), tx.MessagePtr() + tx.MessageSize);

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
		CATAPULT_LOG(debug) << "     Signer: " << FormatKey(tx.Signer);
		CATAPULT_LOG(debug) << "  Signature:\n" << HexFormat(tx.Signature);
		CATAPULT_LOG(debug) << "        Fee: " << VALANDHEX(tx.Fee);
		CATAPULT_LOG(debug) << "   Deadline: " << VALANDHEX(tx.Deadline);

		if (EntityType::Transfer == tx.Type)
			EntityDump(static_cast<const TransferTransaction&>(tx));
	}

	void EntityDump(const Block& block) {
		CATAPULT_LOG(debug) << "        Size: " << VALANDHEX(block.Size);
		CATAPULT_LOG(debug) << "     Version: " << VALANDHEX(block.Version);
		CATAPULT_LOG(debug) << "        Type: " << VALANDHEX(block.Type);
		CATAPULT_LOG(debug) << "      Signer: " << FormatKey(block.Signer);
		CATAPULT_LOG(debug) << "   Timestamp: " << VALANDHEX(block.Timestamp);
		CATAPULT_LOG(debug) << "   Signature:\n" << HexFormat(block.Signature);
		CATAPULT_LOG(debug) << "      Height: " << VALANDHEX(block.Height);
		CATAPULT_LOG(debug) << "  Difficulty: " << VALANDHEX(block.Difficulty);
		CATAPULT_LOG(debug) << " PrevBlockHa: " << HexFormat(block.PreviousBlockHash);
	}

#undef VALANDHEX
}}
