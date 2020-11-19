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

#include "FileBlockChangeStorage.h"
#include "catapult/io/BlockElementSerializer.h"
#include "catapult/io/BlockStatementSerializer.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"

namespace catapult { namespace filespooling {

	namespace {
		class FileBlockChangeStorage final : public io::BlockChangeSubscriber {
		public:
			explicit FileBlockChangeStorage(std::unique_ptr<io::OutputStream>&& pOutputStream) : m_pOutputStream(std::move(pOutputStream))
			{}

		public:
			void notifyBlock(const model::BlockElement& blockElement) override {
				io::Write8(*m_pOutputStream, utils::to_underlying_type(subscribers::BlockChangeOperationType::Block));
				io::WriteBlockElement(blockElement, *m_pOutputStream);
				if (blockElement.OptionalStatement) {
					io::Write8(*m_pOutputStream, 0xFF);
					io::WriteBlockStatement(*blockElement.OptionalStatement, *m_pOutputStream);
				} else {
					io::Write8(*m_pOutputStream, 0);
				}

				m_pOutputStream->flush();
			}

			void notifyDropBlocksAfter(Height height) override {
				io::Write8(*m_pOutputStream, utils::to_underlying_type(subscribers::BlockChangeOperationType::Drop_Blocks_After));
				io::Write(*m_pOutputStream, height);
				m_pOutputStream->flush();
			}

		private:
			std::unique_ptr<io::OutputStream> m_pOutputStream;
		};
	}

	std::unique_ptr<io::BlockChangeSubscriber> CreateFileBlockChangeStorage(std::unique_ptr<io::OutputStream>&& pOutputStream) {
		return std::make_unique<FileBlockChangeStorage>(std::move(pOutputStream));
	}
}}
