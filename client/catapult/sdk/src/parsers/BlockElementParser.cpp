#include "BlockElementParser.h"

namespace catapult { namespace parsers {

	namespace {
		class BufferReader {
		public:
			explicit BufferReader(const RawBuffer& buffer)
					: m_buffer(buffer)
					, m_offset(0)
			{}

		public:
			size_t numBytesConsumed() const {
				return m_offset;
			}

		public:
			template<typename TEntity>
			const TEntity& readEntity() {
				const auto& entity = read<TEntity>();
				if (entity.Size < sizeof(TEntity))
					CATAPULT_THROW_RUNTIME_ERROR("entity size must be at least entity header size");

				auto entityPayloadSize = entity.Size - sizeof(TEntity);
				if (m_buffer.Size - m_offset < entityPayloadSize)
					CATAPULT_THROW_RUNTIME_ERROR("entity does not completely fit in buffer");

				m_offset += entityPayloadSize;
				return entity;
			}

			template<typename TValue>
			const TValue& read() {
				m_offset += sizeof(TValue);
				if (m_offset > m_buffer.Size)
					CATAPULT_THROW_RUNTIME_ERROR("value does not completely fit in buffer");

				return reinterpret_cast<const TValue&>(m_buffer.pData[m_offset - sizeof(TValue)]);
			}

		private:
			RawBuffer m_buffer;
			size_t m_offset;
		};
	}

	model::BlockElement ParseBlockElement(const RawBuffer& buffer, size_t& numBytesConsumed) {
		BufferReader reader(buffer);

		// 1. read full block (including transactions)
		model::BlockElement element(reader.readEntity<model::Block>());

		// 2. read block metadata
		element.EntityHash = reader.read<Hash256>();
		element.GenerationHash = reader.read<Hash256>();

		// 3. write transaction metadata
		for (const auto& transaction : element.Block.Transactions()) {
			element.Transactions.emplace_back(transaction);
			element.Transactions.back().EntityHash = reader.read<Hash256>();
			element.Transactions.back().MerkleComponentHash = reader.read<Hash256>();
		}

		numBytesConsumed = reader.numBytesConsumed();
		return element;
	}
}}
