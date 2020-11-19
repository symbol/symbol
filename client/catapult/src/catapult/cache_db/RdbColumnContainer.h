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

#pragma once
#include "catapult/exceptions.h"
#include "catapult/functions.h"
#include "catapult/types.h"

namespace catapult {
	namespace cache {
		class RdbDataIterator;
		class RocksDatabase;
	}
}

namespace catapult { namespace cache {

	/// RocksDb-backed container adapter.
	class RdbColumnContainer {
	public:
		/// Creates an adapter around \a database and \a columnId.
		RdbColumnContainer(RocksDatabase& database, size_t columnId);

	public:
		/// Gets the size of the column.
		size_t size() const;

		/// Sets the size of the column to \a newSize.
		void setSize(size_t newSize);

		/// Returns \c false if property value of a column (\a propertyName) is not found,
		/// otherwise returns \c true and sets \a value to retrieved property value.
		template<typename TValue>
		bool prop(const std::string& propertyName, TValue& value) const {
			bool result = false;
			load(propertyName, [&result, &value](const char* buffer) {
				if (!buffer)
					return;

				result = true;
				value = reinterpret_cast<const TValue&>(*buffer);
			});

			return result;
		}

		/// Sets the property value of a column (\a propertyName) to \a value.
		template<typename TValue>
		void setProp(const std::string& propertyName, const TValue& value) {
			std::string strValue(sizeof(TValue), 0);
			reinterpret_cast<TValue&>(strValue[0]) = value;
			save(propertyName, strValue);
		}

		/// Gets the underlying database.
		RocksDatabase& database() {
			return m_database;
		}

	protected:
		/// Finds element with \a key, storing result in \a iterator.
		void find(const RawBuffer& key, RdbDataIterator& iterator) const;

		/// Inserts element with \a key and \a value.
		void insert(const RawBuffer& key, const std::string& value);

		/// Removes element with \a key.
		void remove(const RawBuffer& key);

		/// Prunes elements below \a pruningBoundary. Returns number of pruned elements.
		size_t prune(uint64_t pruningBoundary);

	private:
		void load(const std::string& propertyName, const consumer<const char*>& sink) const;

		void save(const std::string& propertyName, const std::string& strValue);

	private:
		RocksDatabase& m_database;
		size_t m_columnId;
		size_t m_size;
	};
}}
