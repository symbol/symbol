#include "AccountImportance.h"
#include <algorithm>

namespace catapult { namespace state {

	// region const_iterator

	AccountImportance::const_iterator::const_iterator(const SnapshotArray* pArray, size_t index)
			: m_pArray(pArray)
			, m_index(index)
	{}

	bool AccountImportance::const_iterator::operator==(const const_iterator& rhs) const {
		return m_pArray == rhs.m_pArray && m_index == rhs.m_index;
	}

	bool AccountImportance::const_iterator::operator!=(const const_iterator& rhs) const {
		return !(*this == rhs);
	}

	AccountImportance::const_iterator& AccountImportance::const_iterator::operator++() {
		if (isEnd())
			CATAPULT_THROW_OUT_OF_RANGE("cannot advance iterator beyond end");

		++m_index;
		return *this;
	}

	AccountImportance::const_iterator AccountImportance::const_iterator::operator++(int) {
		auto copy = *this;
		++*this;
		return copy;
	}

	AccountImportance::const_iterator::reference AccountImportance::const_iterator::operator*() const {
		return *(this->operator->());
	}

	AccountImportance::const_iterator::pointer AccountImportance::const_iterator::operator->() const {
		if (isEnd())
			CATAPULT_THROW_OUT_OF_RANGE("cannot dereference at end");

		return m_pArray ? &(*m_pArray)[m_index] : &m_defaultSnapshot;
	}

	bool AccountImportance::const_iterator::isEnd() const {
		return Importance_History_Size == m_index;
	}

	// endregion

	AccountImportance::AccountImportance() = default;

	AccountImportance::AccountImportance(const AccountImportance& accountImportance) {
		*this = accountImportance;
	}

	AccountImportance::AccountImportance(AccountImportance&& accountImportance) = default;

	AccountImportance& AccountImportance::operator=(const AccountImportance& accountImportance) {
		if (accountImportance.m_pSnapshots)
			m_pSnapshots = std::make_unique<SnapshotArray>(*accountImportance.m_pSnapshots);

		return *this;
	}

	AccountImportance& AccountImportance::operator=(AccountImportance&& accountImportance) = default;

	Importance AccountImportance::current() const {
		return m_pSnapshots ? m_pSnapshots->front().Importance : Importance();
	}

	model::ImportanceHeight AccountImportance::height() const {
		return m_pSnapshots ? m_pSnapshots->front().Height : model::ImportanceHeight();
	}

	Importance AccountImportance::get(model::ImportanceHeight height) const {
		if (!m_pSnapshots)
			return Importance();

		auto iter = std::find_if(m_pSnapshots->cbegin(), m_pSnapshots->cend(), [height](const auto& snapshot) {
			return snapshot.Height == height;
		});

		return m_pSnapshots->cend() == iter ? Importance() : iter->Importance;
	}

	AccountImportance::const_iterator AccountImportance::begin() const {
		return const_iterator(m_pSnapshots.get(), 0);
	}

	AccountImportance::const_iterator AccountImportance::end() const {
		return const_iterator(m_pSnapshots.get(), Importance_History_Size);
	}

	void AccountImportance::set(Importance importance, model::ImportanceHeight height) {
		auto lastHeight = this->height();
		if (lastHeight >= height)
			CATAPULT_THROW_RUNTIME_ERROR_2("new importance must have higher height", height, lastHeight);

		shiftRight();

		if (!m_pSnapshots)
			m_pSnapshots = std::make_unique<SnapshotArray>();

		m_pSnapshots->front() = ImportanceSnapshot(importance, height);
	}

	void AccountImportance::pop() {
		shiftLeft();
		m_pSnapshots->back() = ImportanceSnapshot();

		if (Importance() == current())
			m_pSnapshots.reset();
	}

	void AccountImportance::shiftLeft() {
		if (!m_pSnapshots)
			CATAPULT_THROW_OUT_OF_RANGE("cannot pop when no importances are set");

		auto& snapshots = *m_pSnapshots;
		for (auto i = 0u; i < snapshots.size() - 1; ++i)
			snapshots[i] = snapshots[i + 1];
	}

	void AccountImportance::shiftRight() {
		if (!m_pSnapshots)
			return;

		auto& snapshots = *m_pSnapshots;
		for (auto i = snapshots.size() - 1; i > 0; --i)
			snapshots[i] = snapshots[i - 1];
	}
}}
