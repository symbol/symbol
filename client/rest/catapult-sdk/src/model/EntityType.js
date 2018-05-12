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

/**
 * Catapult model entity types.
 * @enum {numeric}
 * @exports model/EntityType
 */
const EntityType = {
	/** Transfer transaction. */
	transfer: 0x4154,

	/** Register namespace transaction. */
	registerNamespace: 0x414E,

	/** Mosaic definition transaction. */
	mosaicDefinition: 0x414D,

	/** Mosaic supply change transaction. */
	mosaicSupplyChange: 0x424D,

	/** Mosaic levy change transaction. */
	mosaicLevyChange: 0x434D,

	/** Modify multisig account transaction. */
	modifyMultisigAccount: 0x4155,

	/** Aggregate complete transaction. */
	aggregateComplete: 0x4141,

	/** Aggregate bonded transaction. */
	aggregateBonded: 0x4241,

	/** Hash lock transaction. */
	hashLock: 0x414C,

	/** Secret lock transaction. */
	secretLock: 0x424C,

	/** Secret proof transaction. */
	secretProof: 0x434C
};

module.exports = EntityType;
