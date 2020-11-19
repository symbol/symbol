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
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <string>

namespace catapult { namespace mongo {

	/// Represents a mongo database.
	class MongoDatabase {
	public:
		/// Creates a mongo database around \a connectionPool and \a databaseName.
		MongoDatabase(mongocxx::pool& connectionPool, const std::string& databaseName)
				: m_pConnection(connectionPool.acquire())
				, m_database(m_pConnection->database(databaseName))
		{}

	public:
		/// Gets a const reference to the underlying mongocxx database.
		operator const mongocxx::database&() const {
			return m_database;
		}

		/// Gets a reference to the underlying mongocxx database.
		operator mongocxx::database&() {
			return m_database;
		}

		/// Gets the const mongocxx collection with name \a collectionName.
		const mongocxx::collection operator[](const std::string& collectionName) const {
			return m_database[collectionName];
		}

		/// Gets the mongocxx collection with name \a collectionName.
		mongocxx::collection operator[](const std::string& collectionName) {
			return m_database[collectionName];
		}

	private:
		mongocxx::pool::entry m_pConnection;
		mongocxx::database m_database;
	};
}}
