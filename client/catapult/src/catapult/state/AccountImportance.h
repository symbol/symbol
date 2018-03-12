#pragma once
#include "catapult/model/ImportanceHeight.h"
#include "catapult/constants.h"
#include "catapult/exceptions.h"
#include "catapult/types.h"

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

	private:
		using SnapshotArray = std::array<ImportanceSnapshot, Importance_History_Size>;

	public:
		/// The importance const iterator.
		/// \note Iterator will always return three snapshots to mimic std::array behavior.
		/// \note Custom iterator is needed in order to support iteration when std::array member is \c nullptr (memory optimization).
		class const_iterator {
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = const ImportanceSnapshot;
			using pointer = const ImportanceSnapshot*;
			using reference = const ImportanceSnapshot&;
			using iterator_category = std::forward_iterator_tag;

		public:
			/// Creates an iterator around \a pArray with \a index current position.
			explicit const_iterator(const SnapshotArray* pArray, size_t index);

		public:
			/// Returns \c true if this iterator and \a rhs are equal.
			bool operator==(const const_iterator& rhs) const;

			/// Returns \c true if this iterator and \a rhs are not equal.
			bool operator!=(const const_iterator& rhs) const;

		public:
			/// Advances the iterator to the next position.
			const_iterator& operator++();

			/// Advances the iterator to the next position.
			const_iterator operator++(int);

		public:
			/// Returns a reference to the current value.
			reference operator*() const;

			/// Returns a pointer to the current value.
			pointer operator->() const;

		private:
			bool isEnd() const;

		private:
			const SnapshotArray* m_pArray;
			size_t m_index;
			ImportanceSnapshot m_defaultSnapshot;
		};

	public:
		/// Creates an empty account importance.
		AccountImportance();

		/// Copy constructor that makes a deep copy of \a accountImportance.
		AccountImportance(const AccountImportance& accountImportance);

		/// Move constructor that move constructs an account importance from \a accountImportance.
		AccountImportance(AccountImportance&& accountImportance);

	public:
		/// Assignment operator that makes a deep copy of \a accountImportance.
		AccountImportance& operator=(const AccountImportance& accountImportance);

		/// Move assignment operator that assigns \a accountImportance.
		AccountImportance& operator=(AccountImportance&& accountImportance);

	public:
		/// Gets the current importance of the account.
		Importance current() const;

		/// Gets the height at which the current importance was calculated.
		model::ImportanceHeight height() const;

		/// Gets the importance of the account at \a height.
		Importance get(model::ImportanceHeight height) const;

	public:
		/// Returns a const iterator to the first element of the underlying container.
		const_iterator begin() const;

		/// Returns a const iterator to the element following the last element of the underlying container.
		const_iterator end() const;

	public:
		/// Sets the current account importance to \a importance at \a height.
		void set(Importance importance, model::ImportanceHeight height);

		/// Pops the current importance.
		void pop();

	private:
		void shiftLeft();

		void shiftRight();

	private:
		std::unique_ptr<SnapshotArray> m_pSnapshots;
	};
}}
