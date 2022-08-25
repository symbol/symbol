const { deepCompare } = require('./arrayHelpers');
const tweetnacl = require('tweetnacl');

/**
 * Determines if the encoded S part of a signature is canonical.
 * @param {Uint8Array} encodedS Encoded S part of a signature.
 * @returns {boolean} true if the encoded S part is canonical; false otherwise.
 */
const isCanonicalS = encodedS => {
	const reduce = r => {
		const x = new Float64Array(64);
		let i;
		for (i = 0; 64 > i; i++)
			x[i] = r[i];

		for (i = 0; 64 > i; i++)
			r[i] = 0;

		tweetnacl.lowlevel.modL(r, x);
	};

	// require canonical signature
	const reducedEncodedS = new Uint8Array([...encodedS, ...new Uint8Array(32)]);
	reduce(reducedEncodedS);
	return 0 === deepCompare(encodedS, reducedEncodedS.subarray(0, 32));
};

module.exports = { isCanonicalS };
