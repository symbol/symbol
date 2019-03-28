/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @module modelBinary/transactionCodec */

const transactionCodec = {
	/**
	 * Parses a transaction.
	 * @param {object} parser The parser.
	 * @returns {object} The parsed transaction.
	 */
	deserialize: parser => {
		const transaction = {};
		transaction.maxFee = parser.uint64();
		transaction.deadline = parser.uint64();
		return transaction;
	},

	/**
	 * Serializes a transaction.
	 * @param {object} transaction The transaction.
	 * @param {object} serializer The serializer.
	 */
	serialize: (transaction, serializer) => {
		serializer.writeUint64(transaction.maxFee);
		serializer.writeUint64(transaction.deadline);
	}
};

module.exports = transactionCodec;
