#include "Elements.h"

namespace catapult { namespace model {

	namespace {
		class ConditionalEntityInfosBuilder {
		public:
			ConditionalEntityInfosBuilder(WeakEntityInfos& entityInfos, const MatchingEntityPredicate& predicate)
					: m_entityInfos(entityInfos)
					, m_predicate(predicate)
			{}

		public:
			template<typename TElement>
			void add(const TElement& element) {
				const auto& entity = GetEntity(element);
				if (m_predicate(ToBasicEntityType(entity.Type), GetTimestamp(element), element.EntityHash))
					m_entityInfos.push_back(WeakEntityInfo(entity, element.EntityHash));
			}

		private:
			inline
			static const VerifiableEntity& GetEntity(const BlockElement& element) {
				return element.Block;
			}

			inline
			static const VerifiableEntity& GetEntity(const TransactionElement& element) {
				return element.Transaction;
			}

			inline
			static const Timestamp GetTimestamp(const BlockElement& element) {
				return element.Block.Timestamp;
			}

			inline
			static const Timestamp GetTimestamp(const TransactionElement& element) {
				return element.Transaction.Deadline;
			}

		private:
			WeakEntityInfos& m_entityInfos;
			MatchingEntityPredicate m_predicate;
		};

		void AddBlockElement(ConditionalEntityInfosBuilder& builder, const BlockElement& element) {
			for (const auto& transactionElement : element.Transactions)
				builder.add(transactionElement);

			// block element must be added last
			builder.add(element);
		}
	}

	void ExtractMatchingEntityInfos(
			const std::vector<BlockElement>& elements,
			WeakEntityInfos& entityInfos,
			const MatchingEntityPredicate& predicate) {
		ConditionalEntityInfosBuilder builder(entityInfos, predicate);
		for (const auto& element : elements)
			AddBlockElement(builder, element);
	}

	void ExtractEntityInfos(const BlockElement& element, WeakEntityInfos& entityInfos) {
		ConditionalEntityInfosBuilder builder(entityInfos, [](auto, auto, const auto&) { return true; });
		AddBlockElement(builder, element);
	}

	void ExtractTransactionInfos(
			std::vector<TransactionInfo>& transactionInfos,
			const std::shared_ptr<const BlockElement>& pBlockElement) {
		for (const auto& transactionElement : pBlockElement->Transactions) {
			// tie the lifetime of the transaction to the block element
			auto pTransaction = std::shared_ptr<const Transaction>(&transactionElement.Transaction, [pBlockElement](const auto*) {});
			transactionInfos.push_back(MakeTransactionInfo(pTransaction, transactionElement));
		}
	}

	model::TransactionInfo MakeTransactionInfo(
			const std::shared_ptr<const Transaction>& pTransaction,
			const model::TransactionElement& transactionElement) {
		model::TransactionInfo transactionInfo(pTransaction, transactionElement.EntityHash);
		transactionInfo.MerkleComponentHash = transactionElement.MerkleComponentHash;
		transactionInfo.OptionalExtractedAddresses = transactionElement.OptionalExtractedAddresses;
		return transactionInfo;
	}
}}
