#pragma once
#include "catapult/io/BlockStorage.h"

namespace catapult { namespace mocks {

	/// A mock block storage that only supports chain height accesses.
	class MockHeightOnlyBlockStorage : public io::BlockStorage {
	public:
		/// Creates the storage with height \a chainHeight.
		explicit MockHeightOnlyBlockStorage(Height chainHeight) : m_chainHeight(chainHeight)
		{}

	public:
		Height chainHeight() const override {
			return m_chainHeight;
		}

		model::HashRange loadHashesFrom(Height, size_t) const override {
			CATAPULT_THROW_RUNTIME_ERROR("loadHashesFrom - not supported in mock");
		}

		void saveBlock(const model::BlockElement&) override {
			CATAPULT_THROW_RUNTIME_ERROR("saveBlock - not supported in mock");
		}

		void dropBlocksAfter(Height) override {
			CATAPULT_THROW_RUNTIME_ERROR("dropBlocksAfter - not supported in mock");
		}

		std::shared_ptr<const model::Block> loadBlock(Height) const override {
			CATAPULT_THROW_RUNTIME_ERROR("loadBlock - not supported in mock");
		}

		std::shared_ptr<const model::BlockElement> loadBlockElement(Height) const override {
			CATAPULT_THROW_RUNTIME_ERROR("loadBlockElement - not supported in mock");
		}

	private:
		Height m_chainHeight;
	};

	/// A mock block storage that captures saved blocks.
	class MockSavingBlockStorage : public MockHeightOnlyBlockStorage {
	public:
		using MockHeightOnlyBlockStorage::MockHeightOnlyBlockStorage;

	public:
		/// Gets all saved block elements.
		const std::vector<model::BlockElement>& savedBlockElements() const {
			return m_savedBlockElements;
		}

	public:
		void saveBlock(const model::BlockElement& blockElement) override {
			m_savedBlockElements.push_back(blockElement);
		}

	private:
		std::vector<model::BlockElement> m_savedBlockElements;
	};
}}
