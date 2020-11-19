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
#include "catapult/io/BlockChangeSubscriber.h"
#include "tests/test/core/EntityTestUtils.h"
#include <vector>

namespace catapult { namespace mocks {

	/// Mock block change subscriber implementation.
	class MockBlockChangeSubscriber : public io::BlockChangeSubscriber {
	public:
		/// Gets the captured block element pointers.
		const auto& blockElements() const {
			return m_blockElements;
		}

		/// Gets the copied captured block elements.
		const auto& copiedBlockElements() const {
			return m_copiedBlockElements;
		}

		/// Gets the captured drop blocks after heights.
		const auto& dropBlocksAfterHeights() const {
			return m_dropBlocksAfterHeights;
		}

	public:
		void notifyBlock(const model::BlockElement& blockElement) override {
			m_blockElements.push_back(&blockElement);
			m_copiedBlockElements.push_back(copy(blockElement));
		}

		void notifyDropBlocksAfter(Height height) override {
			m_dropBlocksAfterHeights.push_back(height);
		}

	private:
		std::unique_ptr<model::BlockElement> copy(const model::BlockElement& blockElement) {
			// notice that this only copies block parts of blockElement (it does not copy Transactions)
			m_copiedBlocks.push_back(test::CopyEntity(blockElement.Block));
			auto pBlockElementCopy = std::make_unique<model::BlockElement>(*m_copiedBlocks.back());
			pBlockElementCopy->EntityHash = blockElement.EntityHash;
			pBlockElementCopy->GenerationHash = blockElement.GenerationHash;
			pBlockElementCopy->SubCacheMerkleRoots = blockElement.SubCacheMerkleRoots;
			pBlockElementCopy->OptionalStatement = blockElement.OptionalStatement;
			return pBlockElementCopy;
		}

	private:
		std::vector<const model::BlockElement*> m_blockElements;
		std::vector<std::unique_ptr<model::Block>> m_copiedBlocks;
		std::vector<std::unique_ptr<model::BlockElement>> m_copiedBlockElements;
		std::vector<Height> m_dropBlocksAfterHeights;
	};
}}
