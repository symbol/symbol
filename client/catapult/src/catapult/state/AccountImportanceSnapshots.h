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
#include "CompactArrayStack.h"
#include "catapult/model/HeightGrouping.h"

namespace catapult { namespace state {

	/// Stack of account importance snapshots.
	class AccountImportanceSnapshots {
	public:
		/// Temporal importance information.
		struct ImportanceSnapshot {
			/// Account importance.
			catapult::Importance Importance;

			/// Importance height.
			model::ImportanceHeight Height;
		};

	private:
		using SnapshotStack = CompactArrayStack<ImportanceSnapshot, Importance_History_Size>;

	public:
		/// Gets the current importance of the account.
		Importance current() const;

		/// Gets the height at which the current importance was calculated.
		model::ImportanceHeight height() const;

		/// Gets the importance of the account at \a height.
		Importance get(model::ImportanceHeight height) const;

	public:
		/// Sets the current account importance to \a importance at \a height.
		void set(Importance importance, model::ImportanceHeight height);

		/// Pops the current importance.
		void pop();

	public:
		/// Gets a const iterator to the first element of the underlying container.
		SnapshotStack::const_iterator begin() const;

		/// Gets a const iterator to the element following the last element of the underlying container.
		SnapshotStack::const_iterator end() const;

	private:
		SnapshotStack m_snapshots;
	};
}}
