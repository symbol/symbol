#include "ZeroMqPtChangeSubscriber.h"
#include "ZeroMqEntityPublisher.h"

namespace catapult { namespace zeromq {

	namespace {
		class ZeroMqPtChangeSubscriber : public cache::PtChangeSubscriber {
		public:
			explicit ZeroMqPtChangeSubscriber(ZeroMqEntityPublisher& publisher) : m_publisher(publisher)
			{}

		public:
			void notifyAddPartials(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransaction(TransactionMarker::Partial_Transaction_Add_Marker, transactionInfo, Height());
			}

			void notifyAddCosignature(
					const model::TransactionInfo& parentTransactionInfo,
					const Key& signer,
					const Signature& signature) override {
				m_publisher.publishCosignature(parentTransactionInfo, signer, signature);
			}

			void notifyRemovePartials(const TransactionInfos& transactionInfos) override {
				for (const auto& transactionInfo : transactionInfos)
					m_publisher.publishTransactionHash(TransactionMarker::Partial_Transaction_Remove_Marker, transactionInfo);
			}

			void flush() override {
				// empty because messages are pushed by other calls
			}

		private:
			ZeroMqEntityPublisher& m_publisher;
		};
	}

	std::unique_ptr<cache::PtChangeSubscriber> CreateZeroMqPtChangeSubscriber(ZeroMqEntityPublisher& publisher) {
		return std::make_unique<ZeroMqPtChangeSubscriber>(publisher);
	}
}}
