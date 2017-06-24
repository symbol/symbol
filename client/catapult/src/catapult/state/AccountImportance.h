#pragma once
#include "catapult/model/ImportanceHeight.h"
#include "catapult/constants.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"
#include <algorithm>

namespace catapult { namespace state {

	/// Account importance data.
	class AccountImportance {
	public:
		/// Temporal importance information.
		struct ImportanceSnapshot {
		public:
			/// Creates a default importance snapshot.
			constexpr ImportanceSnapshot() = default;

			/// Creates an importance snapshot around \a importance and \a height.
			constexpr ImportanceSnapshot(catapult::Importance importance, model::ImportanceHeight height)
					: Importance(importance)
					, Height(height)
			{}

		public:
			/// The importance.
			catapult::Importance Importance;

			/// The importance height.
			model::ImportanceHeight Height;
		};

	public:
		/// Gets the current importance of the account.
		Importance current() const {
			return m_snapshots.front().Importance;
		}

		/// Gets the height at which the current importance was calculated.
		model::ImportanceHeight height() const {
			return m_snapshots.front().Height;
		}

		/// Gets the importance of the account at \a height.
		Importance get(model::ImportanceHeight height) const {
			auto iter = std::find_if(m_snapshots.cbegin(), m_snapshots.cend(), [height](const auto& snapshot) {
				return snapshot.Height == height;
			});

			return m_snapshots.cend() == iter ? Importance() : iter->Importance;
		}

	public:
		/// Returns a const iterator to the first element of the underlying container
		auto begin() const {
			return m_snapshots.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying container.
		auto end() const {
			return m_snapshots.cend();
		}

	public:
		/// Sets the current account importance to \a importance at \a height.
		void set(Importance importance, model::ImportanceHeight height) {
			auto lastHeight = this->height();
			if (lastHeight >= height)
				CATAPULT_THROW_RUNTIME_ERROR_2("new importance must have higher height", height, lastHeight);

			shiftRight();
			m_snapshots.front() = ImportanceSnapshot(importance, height);
		}

		/// Pops the current importance.
		void pop() {
			shiftLeft();
			m_snapshots.back() = ImportanceSnapshot();
		}

	private:
		void shiftLeft() {
			for (auto i = 0u; i < m_snapshots.size() - 1; ++i)
				m_snapshots[i] = m_snapshots[i + 1];
		}

		void shiftRight() {
			for (auto i = m_snapshots.size() - 1; i > 0; --i)
				m_snapshots[i] = m_snapshots[i - 1];
		}

	private:
		std::array<ImportanceSnapshot, Importance_History_Size> m_snapshots;
	};
}}
