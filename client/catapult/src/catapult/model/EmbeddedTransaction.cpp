#include "EmbeddedTransaction.h"
#include "NotificationSubscriber.h"
#include "Transaction.h"
#include "TransactionPlugin.h"

namespace catapult { namespace model {

	std::ostream& operator<<(std::ostream& out, const EmbeddedTransaction& transaction) {
		auto version = static_cast<uint16_t>(transaction.EntityVersion());
		out << "(embedded) " << transaction.Type << " (v" << version << ") with size " << transaction.Size;
		return out;
	}

	namespace {
		bool TryCalculateRealSize(const EmbeddedTransaction& transaction, const TransactionRegistry& registry, uint64_t& realSize) {
			const auto* pPlugin = registry.findPlugin(transaction.Type);
			if (!pPlugin || !pPlugin->supportsEmbedding()) {
				CATAPULT_LOG(warning) << "rejected embedded transaction with type: " << transaction.Type;
				return false;
			}

			realSize = pPlugin->embeddedPlugin().calculateRealSize(transaction);
			return true;
		}
	}

	bool IsSizeValid(const EmbeddedTransaction& transaction, const TransactionRegistry& registry) {
		uint64_t realSize;
		if (!TryCalculateRealSize(transaction, registry, realSize))
			return false;

		if (transaction.Size == realSize)
			return true;

		CATAPULT_LOG(warning) << transaction.Type << " transaction failed size validation with size " << transaction.Size
				<< " (expected " << realSize << ")";
		return false;
	}

	void PublishNotifications(const EmbeddedTransaction& transaction, NotificationSubscriber& sub) {
		sub.notify(AccountPublicKeyNotification(transaction.Signer));
	}
}}
