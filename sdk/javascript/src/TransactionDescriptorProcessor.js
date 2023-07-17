/**
 * Processes and looks up transaction descriptor properties.
 * @note This class is not intended to be used directly.
 */
export default class TransactionDescriptorProcessor {
	/**
	 * Creates a transaction descriptor processor.
	 * @param {object} transactionDescriptor Transaction descriptor.
	 * @param {Map<string, function>} typeParsingRules Type-dependent parsing rules.
	 * @param {function|undefined} typeConverter Converts a generated type to an sdk type (optional).
	 */
	constructor(transactionDescriptor, typeParsingRules, typeConverter = undefined) {
		/**
		 * @private
		 */
		this._transactionDescriptor = transactionDescriptor;

		/**
		 * @private
		 */
		this._typeParsingRules = typeParsingRules;

		/**
		 * Tries to coerce a value to a more appropriate type.
		 * @param {object} value Original value.
		 * @returns {object} Type converted value.
		 * @private
		 */
		this._typeConverter = typeConverter || (value => value);

		/**
		 * @private
		 */
		this._typeHints = {};
	}

	/**
	 * Looks up value and applies type hints.
	 * @param {string} key Key for which to retrieve value.
	 * @returns {object} Value corresponding to key.
	 * @private
	 */
	_lookupValueAndApplyTypeHints(key) {
		if (undefined === this._transactionDescriptor[key])
			throw RangeError(`transaction descriptor does not have attribute ${key}`);

		let value = this._transactionDescriptor[key];
		const typeHint = this._typeHints[key];
		const rule = this._typeParsingRules.get(typeHint);
		if (rule)
			value = rule(value);

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
			? value.map(item => this._typeConverter(item))
			: this._typeConverter(value);
	}

	/**
	 * Copies all descriptor information to a transaction.
	 * @param {object} transaction Transaction to which to copy keys.
	 * @param {Array<string>|undefined} ignoreKeys Keys of descriptor values not to copy (optional).
	 */
	copyTo(transaction, ignoreKeys = undefined) {
		Object.getOwnPropertyNames(this._transactionDescriptor).forEach(key => {
			if (ignoreKeys && -1 !== ignoreKeys.indexOf(key))
				return;

			if (key.endsWith('Computed'))
				throw RangeError(`cannot explicitly set computed field ${key}`);

			if (undefined === transaction[key])
				throw RangeError(`transaction does not have attribute ${key}`);

			const value = this.lookupValue(key);
			transaction[key] = value;
		});
	}

	/**
	 * Sets type hints.
	 * @param {TypeHintsMap|undefined} typeHints New type hints. // eslint-disable-line valid-jsdoc
	 */
	setTypeHints(typeHints) {
		this._typeHints = typeHints || {};
	}
}

// region type declarations

/**
 * Type hints map.
 * @class
 * @typedef {{[key: string]: string}} TypeHintsMap
 */

// endregion
