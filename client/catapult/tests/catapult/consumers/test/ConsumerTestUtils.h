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
#include "catapult/consumers/ConsumerResults.h"
#include "catapult/disruptor/DisruptorConsumer.h"
#include "catapult/disruptor/DisruptorElement.h"
#include "tests/test/other/DisruptorTestUtils.h"

namespace catapult { namespace test {

	// region BasicElementsInputFacade

	/// Basic wrapper that extends the lifetime of a consumer input and implicitly casts it to typed elements.
	template<typename TElements>
	struct BasicElementsInputFacade {
	protected:
		/// Creates a facade around \a input and \a accessor.
		template<typename TElementsAccessor>
		BasicElementsInputFacade(disruptor::ConsumerInput&& input, TElementsAccessor accessor)
				: m_input(std::move(input))
				, m_elements(accessor(m_input))
		{}

	public:
		/// Casts this to the underlying elements.
		operator TElements&() {
			return m_elements;
		}

		/// Gets the element at \a index.
		auto& operator[](size_t index) {
			return m_elements[index];
		}

	public:
		/// Gets the number of elements.
		size_t size() const {
			return m_elements.size();
		}

		/// Gets a const iterator that represents the first element.
		auto begin() const {
			return m_elements.begin();
		}

		/// Gets a const iterator that represents one past the last element.
		auto end() const {
			return m_elements.end();
		}

	private:
		disruptor::ConsumerInput m_input;
		TElements& m_elements;
	};

	// endregion

	// region CreateBlockElements

	/// Wrapper that extends the lifetime of a consumer input and implicitly casts it to block elements.
	struct BlockElementsInputFacade : public BasicElementsInputFacade<disruptor::BlockElements> {
	private:
		using BaseType = BasicElementsInputFacade<disruptor::BlockElements>;

	public:
		/// Creates a facade around \a input.
		explicit BlockElementsInputFacade(disruptor::ConsumerInput&& input)
				: BaseType(std::move(input), [](auto& movedInput) -> disruptor::BlockElements& {
					return movedInput.blocks();
				})
		{}
	};

	/// Creates \a numBlocks block elements.
	BlockElementsInputFacade CreateBlockElements(size_t numBlocks);

	/// Creates block elements composed of the specified \a blocks.
	BlockElementsInputFacade CreateBlockElements(const std::vector<const model::Block*>& blocks);

	// endregion

	// region CreateTransactionElements

	/// Wrapper that extends the lifetime of a consumer input and implicitly casts it to transaction elements.
	struct TransactionElementsInputFacade : public BasicElementsInputFacade<disruptor::TransactionElements> {
	private:
		using BaseType = BasicElementsInputFacade<disruptor::TransactionElements>;

	public:
		/// Creates a facade around \a input.
		explicit TransactionElementsInputFacade(disruptor::ConsumerInput&& input)
				: BaseType(std::move(input), [](auto& movedInput) -> disruptor::TransactionElements& {
					return movedInput.transactions();
				})
		{}
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

	/// Asserts that \a result is completed with \a validationResult.
	void AssertConsumed(const disruptor::ConsumerResult& result, validators::ValidationResult validationResult);

	/// Asserts that \a result is aborted with \a validationResult and \a severity.
	void AssertAborted(
			const disruptor::ConsumerResult& result,
			validators::ValidationResult validationResult,
			disruptor::ConsumerResultSeverity severity);

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
