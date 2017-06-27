/** @module packet/PacketType */

/**
 * Packet types.
 * @enum {numeric}
 */
export default {
	/** A challenge from a server to a client. */
	serverChallenge: 1,

	/** A challenge from a client to a server. */
	clientChallenge: 2,

	/** Blocks have been pushed by a peer. */
	pushBlock: 3,

	/** Transactions have been pushed by an api-node or a peer. */
	pushTransactions: 9
};
