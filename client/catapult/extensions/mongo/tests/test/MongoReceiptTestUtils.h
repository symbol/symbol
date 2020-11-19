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
#include "MapperTestUtils.h"
#include "mongo/src/mappers/MapperInclude.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/model/Elements.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mocks { struct MockReceipt; } }

namespace catapult { namespace test {

	/// Generates \a numStatements transaction statements.
	std::shared_ptr<model::BlockStatement> GenerateRandomOptionalStatement(size_t numStatements);

	/// Calculates merkle tree from \a transactionElements.
	std::vector<Hash256> CalculateMerkleTree(const std::vector<model::TransactionElement>& transactionElements);

	/// Calculates merkle tree from \a blockStatement using only the transaction statements.
	std::vector<Hash256> CalculateMerkleTreeFromTransactionStatements(const model::BlockStatement& blockStatement);

	/// Verifies that model \a source is equal to db entity (\a sourceView); \a message is used to output additional information.
	void AssertEqualSource(const model::ReceiptSource& source, const bsoncxx::document::view& sourceView, const std::string& message = "");

	/// Verifies that model \a receipt is equal to db entity (\a receiptView); \a message is used to output additional information.
	void AssertEqualReceipt(
			const mocks::MockReceipt& receipt,
			const bsoncxx::document::view& receiptView,
			const std::string& message = "");

	/// Verifies that model transaction \a statement at \a height is equal to db entity (\a statementView) and that the db entity has
	/// \a expectedFieldCount fields; \a index is used to output additional information.
	void AssertEqualTransactionStatement(
			const model::TransactionStatement& statement,
			Height height,
			const bsoncxx::document::view& statementView,
			size_t expectedFieldCount,
			size_t index);

	/// Verifies that model address resolution \a statement at \a height is equal to db entity (\a statementView)
	/// and that the db entity has \a expectedFieldCount fields; \a index is used to output additional information.
	void AssertEqualAddressResolutionStatement(
			const model::AddressResolutionStatement& statement,
			Height height,
			const bsoncxx::document::view& statementView,
			size_t expectedFieldCount,
			size_t index);

	/// Verifies that model mosaic resolution \a statement at \a height is equal to db entity (\a statementView)
	/// and that the db entity has \a expectedFieldCount fields; \a index is used to output additional information.
	void AssertEqualMosaicResolutionStatement(
			const model::MosaicResolutionStatement& statement,
			Height height,
			const bsoncxx::document::view& statementView,
			size_t expectedFieldCount,
			size_t index);

	// region traits

	/// Address resolution traits used in tests.
	struct AddressResolutionTraits {
	public:
		/// Resolution statement type.
		using ResolutionStatementType = model::AddressResolutionStatement;

		/// Unresolved type.
		using UnresolvedType = UnresolvedAddress;

		/// Resolved type.
		using ResolvedType = Address;

	public:
		/// Creates an unresolved type around \a i.
		static auto CreateUnresolved(uint8_t i) {
			return UnresolvedAddress{ { { static_cast<uint8_t>(i + 1) } } };
		}

		/// Creates a resolved type around \a i.
		static auto CreateResolved(uint8_t i) {
			return Address{ { static_cast<uint8_t>(i + 10) } };
		}

		/// Gets the unresolved value from \a view.
		static auto GetUnresolved(const bsoncxx::document::view& view) {
			return test::GetUnresolvedAddressValue(view, "unresolved");
		}

		/// Gets the resolved value from \a view.
		static auto GetResolved(const bsoncxx::document::view& view) {
			return test::GetAddressValue(view, "resolved");
		}

	public:
		static constexpr auto AssertResolutionStatement = AssertEqualAddressResolutionStatement;
	};

	/// Mosaic resolution traits used in tests.
	struct MosaicResolutionTraits {
	public:
		/// Resolution statement type.
		using ResolutionStatementType = model::MosaicResolutionStatement;

		/// Unresolved type.
		using UnresolvedType = UnresolvedMosaicId;

		/// Resolved type.
		using ResolvedType = MosaicId;

	public:
		/// Creates an unresolved type around \a i.
		static auto CreateUnresolved(uint8_t i) {
			return UnresolvedMosaicId(static_cast<uint8_t>(i + 1));
		}

		/// Creates a resolved type around \a i.
		static auto CreateResolved(uint8_t i) {
			return MosaicId(static_cast<uint8_t>(i + 10));
		}

		/// Gets the unresolved value from \a view.
		static auto GetUnresolved(const bsoncxx::document::view& view) {
			return UnresolvedMosaicId(test::GetUint64(view, "unresolved"));
		}

		/// Gets the resolved value from \a view.
		static auto GetResolved(const bsoncxx::document::view& view) {
			return MosaicId(test::GetUint64(view, "resolved"));
		}

	public:
		static constexpr auto AssertResolutionStatement = AssertEqualMosaicResolutionStatement;
	};

	// endregion
}}
