#pragma once
#include "DisruptorTypes.h"
#include "InputSource.h"
#include "catapult/model/AnnotatedEntityRange.h"
#include "catapult/model/RangeTypes.h"

namespace catapult { namespace disruptor {

	/// Consumer input composed of a range of entities augmented with metadata.
	class ConsumerInput {
	public:
		/// Creates a default consumer input.
		ConsumerInput() : m_source(InputSource::Unknown)
		{}

		/// Creates a consumer input around a block \a range with an optional input source (\a inputSource).
		explicit ConsumerInput(model::AnnotatedBlockRange&& range, InputSource source = InputSource::Unknown)
				: m_blockRange(std::move(range.Range))
				, m_source(source)
				, m_sourcePublicKey(range.SourcePublicKey) {
			m_blockElements.reserve(m_blockRange.size());
			for (const auto& block : m_blockRange)
				m_blockElements.push_back(model::BlockElement(block));
		}

		/// Creates a consumer input around a transaction \a range with an optional input source (\a inputSource).
		explicit ConsumerInput(model::AnnotatedTransactionRange&& range, InputSource source = InputSource::Unknown)
				: m_transactionRange(std::move(range.Range))
				, m_source(source)
				, m_sourcePublicKey(range.SourcePublicKey) {
			m_transactionElements.reserve(m_transactionRange.size());
			for (const auto& transaction : m_transactionRange)
				m_transactionElements.push_back(FreeTransactionElement(transaction));
		}

	public:
		/// Returns \c true if this input is empty and has no elements.
		bool empty() const {
			return m_blockRange.empty() && m_transactionRange.empty();
		}

		/// Returns \c true if this input is non-empty and has blocks.
		bool hasBlocks() const {
			return !m_blockRange.empty();
		}

		/// Returns \c true if this input is non-empty and has transactions.
		bool hasTransactions() const {
			return !m_transactionRange.empty();
		}

	public:
		/// Returns the block elements associated with this input.
		BlockElements& blocks() {
			if (m_blockElements.empty())
				CATAPULT_THROW_RUNTIME_ERROR("input has no blocks set");

			return m_blockElements;
		}

		/// Returns the const block elements associated with this input.
		const BlockElements& blocks() const {
			return const_cast<ConsumerInput*>(this)->blocks();
		}

		/// Returns the (free) transaction elements associated with this input.
		TransactionElements& transactions() {
			if (m_transactionElements.empty())
				CATAPULT_THROW_RUNTIME_ERROR("input has no transactions set");

			return m_transactionElements;
		}

		/// Returns the const (free) transaction elements associated with this input.
		const TransactionElements& transactions() const {
			return const_cast<ConsumerInput*>(this)->transactions();
		}

		/// Gets the source of this input.
		InputSource source() const {
			return m_source;
		}

		/// Gets the (optional) source public key.
		const Key& sourcePublicKey() const {
			return m_sourcePublicKey;
		}

	public:
		/// Detaches the block range associated with this input.
		model::BlockRange detachBlockRange() {
			if (m_blockRange.empty())
				CATAPULT_THROW_RUNTIME_ERROR("input has no blocks set");

			return std::move(m_blockRange);
		}

		/// Detaches the transaction range associated with this input.
		model::TransactionRange detachTransactionRange() {
			if (m_transactionRange.empty())
				CATAPULT_THROW_RUNTIME_ERROR("input has no transactions set");

			return std::move(m_transactionRange);
		}

	public:
		/// Insertion operator for outputting \a input to \a out.
		friend std::ostream& operator<<(std::ostream& out, const ConsumerInput& input);

	private:
		// backing memory
		model::BlockRange m_blockRange;
		model::TransactionRange m_transactionRange;

		// used by consumers
		BlockElements m_blockElements;
		TransactionElements m_transactionElements;

		InputSource m_source;
		Key m_sourcePublicKey;
	};
}}
