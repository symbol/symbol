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

#include "AccountImportanceSnapshots.h"
#include <algorithm>

namespace catapult { namespace state {

	bool AccountImportanceSnapshots::empty() const {
		return std::all_of(m_snapshots.begin(), m_snapshots.end(), [](const auto& bucket) {
			return model::ImportanceHeight() == bucket.Height;
		});
	}

	bool AccountImportanceSnapshots::active() const {
		return model::ImportanceHeight() != height();
	}

	Importance AccountImportanceSnapshots::current() const {
		return m_snapshots.begin()->Importance;
	}

	model::ImportanceHeight AccountImportanceSnapshots::height() const {
		return m_snapshots.begin()->Height;
	}

	Importance AccountImportanceSnapshots::get(model::ImportanceHeight height) const {
		auto iter = std::find_if(m_snapshots.begin(), m_snapshots.end(), [height](const auto& snapshot) {
			return snapshot.Height == height;
		});

		return m_snapshots.end() == iter ? Importance() : iter->Importance;
	}

	void AccountImportanceSnapshots::set(Importance importance, model::ImportanceHeight height) {
		auto lastHeight = this->height();
		if (model::ImportanceHeight() != lastHeight && lastHeight >= height) {
			std::ostringstream out;
			out << "importances must be set with ascending heights (last = " << lastHeight << ", new = " << height << ")";
			CATAPULT_THROW_RUNTIME_ERROR(out.str().c_str());
		}

		m_snapshots.push({ importance, height });
	}

	void AccountImportanceSnapshots::push() {
		m_snapshots.push(ImportanceSnapshot());
	}

	void AccountImportanceSnapshots::pop() {
		m_snapshots.pop();
	}

	AccountImportanceSnapshots::SnapshotStack::const_iterator AccountImportanceSnapshots::begin() const {
		return m_snapshots.begin();
	}

	AccountImportanceSnapshots::SnapshotStack::const_iterator AccountImportanceSnapshots::end() const {
		return m_snapshots.end();
	}
}}
