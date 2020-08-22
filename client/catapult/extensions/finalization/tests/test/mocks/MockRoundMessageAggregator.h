/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "finalization/src/chain/RoundContext.h"
#include "finalization/src/chain/RoundMessageAggregator.h"
#include "finalization/src/model/FinalizationMessage.h"

namespace catapult { namespace mocks {

	/// Mock round message aggregator.
	class MockRoundMessageAggregator : public chain::RoundMessageAggregator {
	public:
		/// Creates a mock aggregator for \a point and \a height.
		MockRoundMessageAggregator(FinalizationPoint point, Height height)
				: m_point(point)
				, m_height(height)
				, m_numAddCalls(0)
				, m_roundContext(1000, 700)
				, m_addResult(static_cast<chain::RoundMessageAggregatorAddResult>(-1))
		{}

	public:
		/// Gets the finalization point.
		FinalizationPoint point() const {
			return m_point;
		}

		/// Gets the finalization height.
		Height height() const {
			return m_height;
		}

		/// Gets the round context.
		chain::RoundContext& roundContext() {
			return m_roundContext;
		}

		/// Gets the number of times add was called.
		size_t numAddCalls() const {
			return m_numAddCalls;
		}

	public:
		/// Sets the result of shortHashes to \a shortHashes.
		void setShortHashes(model::ShortHashRange&& shortHashes) {
			m_shortHashes = std::move(shortHashes);
		}

		/// Sets the messages filtered by unknownMessages to \a messages.
		void setMessages(UnknownMessages&& messages) {
			m_messages = std::move(messages);
		}

		/// Sets the result of add to \a result.
		void setAddResult(chain::RoundMessageAggregatorAddResult result) {
			m_addResult = result;
		}

	public:
		size_t size() const override {
			CATAPULT_THROW_RUNTIME_ERROR("size - not supported in mock");
		}

		const model::FinalizationContext& finalizationContext() const override {
			CATAPULT_THROW_RUNTIME_ERROR("finalizationContext - not supported in mock");
		}

		const chain::RoundContext& roundContext() const override {
			return m_roundContext;
		}

		model::ShortHashRange shortHashes() const override {
			return model::ShortHashRange::CopyRange(m_shortHashes);
		}

		UnknownMessages unknownMessages(const utils::ShortHashesSet& knownShortHashes) const override {
			UnknownMessages filteredMessages;
			for (const auto& pMessage : m_messages) {
				auto shortHash = utils::ToShortHash(model::CalculateMessageHash(*pMessage));
				if (knownShortHashes.cend() == knownShortHashes.find(shortHash))
					filteredMessages.push_back(pMessage);
			}

			return filteredMessages;
		}

	public:
		chain::RoundMessageAggregatorAddResult add(const std::shared_ptr<model::FinalizationMessage>&) override {
			++m_numAddCalls;
			return m_addResult;
		}

	private:
		FinalizationPoint m_point;
		Height m_height;
		size_t m_numAddCalls;
		chain::RoundContext m_roundContext;

		model::ShortHashRange m_shortHashes;
		UnknownMessages m_messages;
		chain::RoundMessageAggregatorAddResult m_addResult;
	};
}}
