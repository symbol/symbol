/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
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

/** @module packet/PacketType */

const statePathBaseType = 0x200;

/**
 * Packet types.
 * @enum {numeric}
 */
const PacketType = {
	/** A challenge from a server to a client. */
	serverChallenge: 1,

	/** A challenge from a client to a server. */
	clientChallenge: 2,

	/** Blocks have been pushed by a peer. */
	pushBlock: 3,

	/** Transactions have been pushed by an api-node or a peer. */
	pushTransactions: 9,

	/** Partial aggregate transactions have been pushed by an api-node. */
	pushPartialTransactions: 0x100,

	/** Detached cosignatures have been pushed by an api-node. */
	pushDetachedCosignatures: 0x101,

	/** Node information has been requested by a peer. */
	nodeDiscoveryPullPing: 0x111,

	/** Node peers had been requested by a peer. */
	nodeDiscoveryPullPeers: 0x113,

	/** Node time information has been requested by a peer. */
	timeSyncNodeTime: 0x120,

	/** Finalization proof has been requested by a peer */
	finalizationProofAtEpoch: 0x133,
	finalizationProofAtHeight: 0x134,

	/** State path has been requested by a peer. */
	accountStatePath: statePathBaseType + 0x43,
	hashLockStatePath: statePathBaseType + 0x48,
	secretLockStatePath: statePathBaseType + 0x52,
	metadataStatePath: statePathBaseType + 0x44,
	mosaicStatePath: statePathBaseType + 0x4D,
	multisigStatePath: statePathBaseType + 0x55,
	namespaceStatePath: statePathBaseType + 0x4E,
	accountRestrictionsStatePath: statePathBaseType + 0x50,
	mosaicRestrictionsStatePath: statePathBaseType + 0x51
};

module.exports = {
	PacketType,
	StatePathPacketTypes: [
		PacketType.accountStatePath,
		PacketType.hashLockStatePath,
		PacketType.secretLockStatePath,
		PacketType.metadataStatePath,
		PacketType.mosaicStatePath,
		PacketType.multisigStatePath,
		PacketType.namespaceStatePath,
		PacketType.accountRestrictionsStatePath,
		PacketType.mosaicRestrictionsStatePath
	]
};
