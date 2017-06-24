#pragma once
#include "catapult/model/ChainScore.h"
#include "catapult/utils/SpinReaderWriterLock.h"

namespace catapult { namespace local {

	/// The chain score stored by the local node.
	/// \note This score is synchronized by a reader writer lock.
	class LocalNodeChainScore {
	public:
		/// Creates a default local node chain score.
		LocalNodeChainScore() = default;

		/// Creates a local node chain score around \a score.
		explicit LocalNodeChainScore(const model::ChainScore& score) : m_score (score)
		{}

	public:
		/// Gets the current chain score.
		model::ChainScore get() const {
			auto readLock = m_lock.acquireReader();
			return m_score;
		}

	public:
		/// Adds \a rhs to this chain score.
		LocalNodeChainScore& operator+=(const model::ChainScore& rhs) {
			auto readLock = m_lock.acquireReader();
			auto writeLock = readLock.promoteToWriter();
			m_score += rhs;
			return *this;
		}

	private:
		model::ChainScore m_score;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}
