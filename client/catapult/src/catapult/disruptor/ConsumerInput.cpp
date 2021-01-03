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

#include "ConsumerInput.h"
#include "catapult/utils/HexFormatter.h"
#include <ostream>

namespace catapult { namespace disruptor {

	// region constructors

	ConsumerInput::ConsumerInput() : m_source(InputSource::Unknown)
	{}

	ConsumerInput::ConsumerInput(model::AnnotatedBlockRange&& range, InputSource source)
			: m_blockRange(std::move(range.Range))
			, m_source(source)
			, m_sourceIdentity(range.SourceIdentity) {
		uint64_t memorySize = 0;
		m_blockElements.reserve(m_blockRange.size());
		for (const auto& block : m_blockRange) {
			m_blockElements.push_back(model::BlockElement(block));
			memorySize += block.Size;
		}

		m_memorySize = utils::FileSize::FromBytes(memorySize);

		if (!m_blockElements.empty()) {
			m_startHeight = m_blockElements.front().Block.Height;
			m_endHeight = m_blockElements.back().Block.Height;
		}
	}

	ConsumerInput::ConsumerInput(model::AnnotatedTransactionRange&& range, InputSource source)
			: m_transactionRange(std::move(range.Range))
			, m_source(source)
			, m_sourceIdentity(range.SourceIdentity) {
		uint64_t memorySize = 0;
		m_transactionElements.reserve(m_transactionRange.size());
		for (const auto& transaction : m_transactionRange) {
			m_transactionElements.push_back(FreeTransactionElement(transaction));
			memorySize += transaction.Size;
		}

		m_memorySize = utils::FileSize::FromBytes(memorySize);
	}

	// endregion

	// region predicates

	bool ConsumerInput::empty() const {
		return m_blockRange.empty() && m_transactionRange.empty();
	}

	bool ConsumerInput::hasBlocks() const {
		return !m_blockRange.empty();
	}

	bool ConsumerInput::hasTransactions() const {
		return !m_transactionRange.empty();
	}

	// endregion

	// region accessors

	BlockElements& ConsumerInput::blocks() {
		if (m_blockElements.empty())
			CATAPULT_THROW_RUNTIME_ERROR("input has no blocks set");

		return m_blockElements;
	}

	const BlockElements& ConsumerInput::blocks() const {
		return const_cast<ConsumerInput*>(this)->blocks();
	}

	TransactionElements& ConsumerInput::transactions() {
		if (m_transactionElements.empty())
			CATAPULT_THROW_RUNTIME_ERROR("input has no transactions set");

		return m_transactionElements;
	}

	const TransactionElements& ConsumerInput::transactions() const {
		return const_cast<ConsumerInput*>(this)->transactions();
	}

	InputSource ConsumerInput::source() const {
		return m_source;
	}

	const model::NodeIdentity& ConsumerInput::sourceIdentity() const {
		return m_sourceIdentity;
	}

	utils::FileSize ConsumerInput::memorySize() const {
		return m_memorySize;
	}

	// endregion

	// region detach

	model::BlockRange ConsumerInput::detachBlockRange() {
		if (m_blockRange.empty())
			CATAPULT_THROW_RUNTIME_ERROR("input has no blocks set");

		return std::move(m_blockRange);
	}

	model::TransactionRange ConsumerInput::detachTransactionRange() {
		if (m_transactionRange.empty())
			CATAPULT_THROW_RUNTIME_ERROR("input has no transactions set");

		return std::move(m_transactionRange);
	}

	// endregion

	// region insertion operator

	namespace {
		void OutputShortHash(std::ostream& out, const Hash256& hash) {
			out << " [" << utils::HexFormat(hash.data(), hash.data() + 4) << "] ";
		}

		void OutputAdditionalInformation(std::ostream& out, Height startHeight, Height endHeight) {
			out << " (heights " << startHeight << " - " << endHeight << ")";
		}

		void OutputAdditionalInformation(std::ostream&)
		{}

		template<typename TElements, typename ...TArgs>
		void OutputElementsInfoT(std::ostream& out, const TElements& elements, const char* tag, TArgs... args) {
			if (elements.empty())
				return;

			out << elements.size() << " " << tag;
			OutputAdditionalInformation(out, std::forward<TArgs>(args)...);
			OutputShortHash(out, elements.front().EntityHash); // EntityHash access is ok because it is stored by value in element
		}
	}

	std::ostream& operator<<(std::ostream& out, const ConsumerInput& input) {
		OutputElementsInfoT(out, input.m_blockElements, "blocks", input.m_startHeight, input.m_endHeight);
		OutputElementsInfoT(out, input.m_transactionElements, "txes");

		if (input.empty())
			out << "empty ";

		out << "from " << input.source() << " with size " << input.memorySize();
		return out;
	}

	// endregion
}}
