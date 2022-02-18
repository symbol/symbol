/**
 * Processes and looks up transaction descriptor properties.
 */
class TransactionDescriptorProcessor {
	/**
	 * Creates a transaction descriptor processor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @param {Map} typeParsingRules Type-dependent parsing rules.
	 * @param {function} typeConverter Converts a generated type to an sdk type (optional).
	 */
	constructor(transactionDescriptor, typeParsingRules, typeConverter = undefined) {
		this.transactionDescriptor = transactionDescriptor;
		this.typeParsingRules = typeParsingRules;
		this.typeConverter = typeConverter || (value => value);
		this.typeHints = {};
	}

	_lookupValueAndApplyTypeHints(key) {
		if (undefined === this.transactionDescriptor[key])
			throw RangeError(`transaction descriptor does not have attribute ${key}`);

		let value = this.transactionDescriptor[key];
		const typeHint = this.typeHints[key];
		if (this.typeParsingRules.has(typeHint))
			value = this.typeParsingRules.get(typeHint)(value);

		return value;
	}

	/**
	 * Looks up the value for key.
	 * @param {string} key Key for which to retrieve value.
	 * @returns {object} Value corresponding to key.
	 */
	lookupValue(key) {
		const value = this._lookupValueAndApplyTypeHints(key);
		return Array.isArray(value)
			? value.map(item => this.typeConverter(item))
			: this.typeConverter(value);
	}

	/**
	 * Copies all descriptor information to a transaction.
	 * @param {object} transaction Transaction to which to copy keys.
	 * @param {array<string>} ignoreKeys Keys of descriptor values not to copy (optional).
	 */
	copyTo(transaction, ignoreKeys = undefined) {
		Object.getOwnPropertyNames(this.transactionDescriptor).forEach(key => {
			if (ignoreKeys && -1 !== ignoreKeys.indexOf(key))
				return;

			if (undefined === transaction[key])
				throw RangeError(`transaction does not have attribute ${key}`);

			const value = this.lookupValue(key);
			transaction[key] = value;
		});
	}

	/**
	 * Sets type hints.
	 * @param {object} typeHints New type hints.
	 */
	setTypeHints(typeHints) {
		this.typeHints = typeHints || {};
	}
}

module.exports = { TransactionDescriptorProcessor };
