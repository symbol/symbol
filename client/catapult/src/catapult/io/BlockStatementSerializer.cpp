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

#include "BlockStatementSerializer.h"
#include "PodIoUtils.h"
#include "Stream.h"

namespace catapult { namespace io {

	namespace {
		template<typename T>
		auto ToBytePointer(T& data) {
			using Pointer = std::conditional_t<std::is_const_v<T>, const uint8_t*, uint8_t*>;
			return reinterpret_cast<Pointer>(&data);
		}

		void ReadStatement(InputStream& inputStream, std::map<model::ReceiptSource, model::TransactionStatement>& statements) {
			model::ReceiptSource key;
			inputStream.read({ ToBytePointer(key), sizeof(key) });

			model::TransactionStatement statement(key);
			auto numReceipts = Read32(inputStream);

			constexpr auto Header_Size = sizeof(uint32_t);
			std::vector<uint8_t> receiptBuffer;
			for (auto i = 0u; i < numReceipts; ++i) {
				auto receiptSize = Read32(inputStream);
				receiptBuffer.resize(receiptSize);
				inputStream.read({ receiptBuffer.data() + Header_Size, receiptSize - Header_Size });

				auto& receipt = reinterpret_cast<model::Receipt&>(*receiptBuffer.data());
				receipt.Size = receiptSize;
				statement.addReceipt(receipt);
			}

			statements.emplace(key, std::move(statement));
		}

		template<typename TUnresolvedKey, typename TResolutionStatement>
		void ReadStatement(InputStream& inputStream, std::map<TUnresolvedKey, TResolutionStatement>& statements) {
			TUnresolvedKey key;
			inputStream.read({ ToBytePointer(key), sizeof(key) });

			TResolutionStatement statement(key);
			auto numEntries = Read32(inputStream);

			typename TResolutionStatement::ResolutionEntry entry;
			for (auto i = 0u; i < numEntries; ++i) {
				inputStream.read({ ToBytePointer(entry), sizeof(entry) });
				statement.addResolution(entry.ResolvedValue, entry.Source);
			}

			statements.emplace(key, std::move(statement));
		}

		template<typename TKey, typename TValue>
		void ReadStatements(InputStream& inputStream, std::map<TKey, TValue>& statements) {
			auto numStatements = Read32(inputStream);
			for (auto i = 0u; i < numStatements; ++i)
				ReadStatement(inputStream, statements);
		}
	}

	void ReadBlockStatement(InputStream& inputStream, model::BlockStatement& blockStatement) {
		ReadStatements(inputStream, blockStatement.TransactionStatements);
		ReadStatements(inputStream, blockStatement.AddressResolutionStatements);
		ReadStatements(inputStream, blockStatement.MosaicResolutionStatements);
	}

	namespace {
		void WriteStatement(OutputStream& outputStream, const model::ReceiptSource& key, const model::TransactionStatement& statement) {
			outputStream.write({ ToBytePointer(key), sizeof(key) });

			Write32(outputStream, utils::checked_cast<size_t, uint32_t>(statement.size()));
			for (auto i = 0u; i < statement.size(); ++i) {
				const auto& receipt = statement.receiptAt(i);
				outputStream.write({ ToBytePointer(receipt), receipt.Size });
			}
		}

		template<typename TUnresolvedKey, typename TResolutionStatement>
		void WriteStatement(OutputStream& outputStream, const TUnresolvedKey& key, const TResolutionStatement& statement) {
			outputStream.write({ ToBytePointer(key), sizeof(key) });

			Write32(outputStream, utils::checked_cast<size_t, uint32_t>(statement.size()));
			for (auto i = 0u; i < statement.size(); ++i) {
				const auto& entry = statement.entryAt(i);
				outputStream.write({ ToBytePointer(entry), sizeof(entry) });
			}
		}

		template<typename TKey, typename TValue>
		void WriteStatements(OutputStream& outputStream, const std::map<TKey, TValue>& statements) {
			Write32(outputStream, utils::checked_cast<size_t, uint32_t>(statements.size()));
			for (const auto& pair : statements)
				WriteStatement(outputStream, pair.first, pair.second);
		}
	}

	void WriteBlockStatement(OutputStream& outputStream, const model::BlockStatement& blockStatement) {
		WriteStatements(outputStream, blockStatement.TransactionStatements);
		WriteStatements(outputStream, blockStatement.AddressResolutionStatements);
		WriteStatements(outputStream, blockStatement.MosaicResolutionStatements);
	}
}}
