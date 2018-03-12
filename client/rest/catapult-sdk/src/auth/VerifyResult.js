/** @module auth/VerifyResult */

/**
 * Possible results of a verification handshake with a peer.
 * @enum {numeric}
 */
module.exports = {
	/** The peer was verified. */
	success: 0,

	/** An i/o error was encountered during verification. */
	ioError: 1,

	/** The peer sent malformed data. */
	malformedData: 2,

	/** The peer failed the challenge. */
	failedChallenge: 3
};
