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

/** @module auth/VerifyResult */

/**
 * Possible results of a verification handshake with a peer.
 * @enum {numeric}
 */
const VerifyResult = {
	/** The peer was verified. */
	success: 0,

	/** An i/o error was encountered during verification. */
	ioError: 1,

	/** The peer sent malformed data. */
	malformedData: 2,

	/** The peer failed the challenge. */
	failedChallenge: 3
};

module.exports = VerifyResult;
