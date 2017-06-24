#include "EmbeddedEntity.h"
#include "NotificationSubscriber.h"
#include "Transaction.h"
#include "TransactionPlugin.h"

namespace catapult { namespace model {

	std::ostream& operator<<(std::ostream& out, const EmbeddedEntity& entity) {
		out << "(embedded) " << entity.Type << " (v" << static_cast<uint16_t>(entity.EntityVersion()) << ") with size " << entity.Size;
		return out;
	}

	namespace {
		bool TryCalculateRealSize(const EmbeddedEntity& entity, const TransactionRegistry& registry, uint64_t& realSize) {
			const auto* pPlugin = registry.findPlugin(entity.Type);
			if (!pPlugin || !pPlugin->supportsEmbedding()) {
				CATAPULT_LOG(warning) << "rejected embedded entity with type: " << entity.Type;
				return false;
			}

			realSize = pPlugin->embeddedPlugin().calculateRealSize(entity);
			return true;
		}
	}

	bool IsSizeValid(const EmbeddedEntity& entity, const TransactionRegistry& registry) {
		uint64_t realSize;
		if (!TryCalculateRealSize(entity, registry, realSize))
			return false;

		if (entity.Size == realSize)
			return true;

		CATAPULT_LOG(warning) << entity.Type << " entity failed size validation with size " << entity.Size
				<< " (expected " << realSize << ")";
		return false;
	}

	void PublishNotifications(const EmbeddedEntity& entity, NotificationSubscriber& sub) {
		sub.notify(AccountPublicKeyNotification(entity.Signer));
	}
}}
