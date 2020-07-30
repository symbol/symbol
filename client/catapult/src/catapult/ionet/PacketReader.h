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
#include "Packet.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace ionet {

	/// Stateful packet reader.
	/// \note Behavior is undefined if error has been encountered on any previous call.
	/// \note All reads are done in-place, so caller is responsible for ensuring aligned reads.
	class PacketReader {
	public:
		/// Creates a reader around \a packet.
		explicit PacketReader(const Packet& packet)
				: m_pData(packet.Data())
				, m_numRemainingBytes(packet.Size)
				, m_hasError(false) {
			require(sizeof(PacketHeader), "constructor");
			m_numRemainingBytes -= SizeOf32<PacketHeader>();
		}

	public:
		/// Returns \c true if the reader has consumed all data, \c false otherwise.
		bool empty() const {
			return 0 == m_numRemainingBytes || m_hasError;
		}

		/// Returns \c true if a reading error has been encountered, \c false otherwise.
		bool hasError() const {
			return m_hasError;
		}

	public:
		/// Reads a fixed-sized value from the packet.
		template<typename TValue>
		const TValue* readFixed() {
			require(sizeof(TValue), "readFixed");
			if (hasError())
				return nullptr;

			const auto& value = reinterpret_cast<const TValue&>(*m_pData);
			advance(sizeof(TValue));
			return &value;
		}

		/// Reads a variable-sized value from the packet.
		template<typename TEntity>
		const TEntity* readVariable() {
			auto pSize = readFixed<uint32_t>();
			if (!pSize)
				return nullptr;

			// readFixed above (for size) advances the data pointer past the size, but the variable entity should point to the size
			rewind(sizeof(uint32_t));
			require(*pSize, "readVariable");
			if (hasError())
				return nullptr;

			const auto& entity = reinterpret_cast<const TEntity&>(*m_pData);
			advance(*pSize);
			return &entity;
		}

	private:
		void require(uint32_t numBytes, const char* message) {
			if (m_numRemainingBytes >= numBytes)
				return;

			CATAPULT_LOG(warning) << message << ": requested (" << numBytes << ") bytes with only " << m_numRemainingBytes << " remaining";
			m_hasError = true;
		}

		void advance(uint32_t numBytes) {
			m_pData += numBytes;
			m_numRemainingBytes -= numBytes;
		}

		void rewind(uint32_t numBytes) {
			m_pData -= numBytes;
			m_numRemainingBytes += numBytes;
		}

	private:
		const uint8_t* m_pData;
		uint32_t m_numRemainingBytes;
		bool m_hasError;
	};
}}
