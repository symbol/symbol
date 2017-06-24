#pragma once
#include "catapult/consumers/ConsumerResults.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/disruptor/DisruptorElement.h"
#include "tests/test/other/DisruptorTestUtils.h"

namespace catapult { namespace test {

	// region CreateBlockElements

	/// A wrapper that extends the lifetime of a consumer input and implicitly casts it to block elements.
	struct BlockElementsInputFacade {
	public:
		/// Creates a facade around \a input.
		explicit BlockElementsInputFacade(disruptor::ConsumerInput&& input) : m_input(std::move(input))
		{}

	public:
		/// Casts this to the underlying block elements.
		operator disruptor::BlockElements&() {
			return m_input.blocks();
		}

		/// Gets the block element at \a index.
		auto& operator[](size_t index) {
			return m_input.blocks()[index];
		}

	public:
		/// Gets the number of block elements.
		size_t size() const {
			return m_input.blocks().size();
		}

	private:
		disruptor::ConsumerInput m_input;
	};

	/// Creates \a numBlocks block elements.
	BlockElementsInputFacade CreateBlockElements(size_t numBlocks);

	/// Creates block elements composed of the specified \a blocks.
	BlockElementsInputFacade CreateBlockElements(const std::vector<const model::Block*>& blocks);

	// endregion

	// region CreateTransactionElements

	/// A wrapper that extends the lifetime of a consumer input and implicitly casts it to transaction elements.
	struct TransactionElementsInputFacade {
	public:
		/// Creates a facade around \a input.
		explicit TransactionElementsInputFacade(disruptor::ConsumerInput&& input) : m_input(std::move(input))
		{}

	public:
		/// Casts this to the underlying transaction elements.
		operator disruptor::TransactionElements&() {
			return m_input.transactions();
		}

		/// Gets the transaction element at \a index.
		auto& operator[](size_t index) {
			return m_input.transactions()[index];
		}

	public:
		/// Returns a const iterator that represents the first transaction.
		auto begin() const {
			return m_input.transactions().begin();
		}

		/// Returns a const iterator that represents one past the last transaction.
		auto end() const {
			return m_input.transactions().end();
		}

	private:
		disruptor::ConsumerInput m_input;
	};

	/// Creates \a numTransactions transaction elements.
	TransactionElementsInputFacade CreateTransactionElements(size_t numTransactions);

	/// Creates transaction elements composed of the specified \a transactions.
	TransactionElementsInputFacade CreateTransactionElements(const std::vector<const model::Transaction*>& transactions);

	// endregion

	// region LinkBlocks

	/// Links \a parentBlock and \a childBlock blocks.
	void LinkBlocks(const model::Block& parentBlock, model::Block& childBlock);

	/// Links all \a blockElements starting at \a chainHeight.
	void LinkBlocks(Height chainHeight, disruptor::BlockElements& blockElements);

	// endregion

	// region ConsumerResult Assertions

	/// Asserts that \a result is completed due to consumption.
	void AssertConsumed(const disruptor::ConsumerResult& result);

	/// Asserts that \a result is aborted with \a validationResult.
	void AssertAborted(const disruptor::ConsumerResult& result, validators::ValidationResult validationResult);

	// endregion

	// region AssertPassthroughForEmptyInput

	/// Asserts that passing an empty input to \a consumer has no effect.
	void AssertPassthroughForEmptyInput(const disruptor::BlockConsumer& consumer);

	/// Asserts that passing an empty input to \a consumer has no effect.
	void AssertPassthroughForEmptyInput(const disruptor::TransactionConsumer& consumer);

	/// Asserts that passing an empty input to \a consumer has no effect.
	void AssertPassthroughForEmptyInput(const disruptor::DisruptorConsumer& consumer);

	// endregion
}}
