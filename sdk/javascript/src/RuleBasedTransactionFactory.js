import BaseValue from './BaseValue.js';
import ByteArray from './ByteArray.js';
import TransactionDescriptorProcessor from './TransactionDescriptorProcessor.js';

const buildEnumStringToValueMap = EnumClass => new Map(Object.getOwnPropertyNames(EnumClass)
	.filter(name => name.toUpperCase() === name)
	.map(name => [name.toLowerCase(), EnumClass[name]]));

const nameToEnumValue = (mapping, enumType, enumValueName) => {
	if (!mapping.has(enumValueName))
		throw RangeError(`unknown value ${enumValueName} for type ${enumType}`);

	return mapping.get(enumValueName);
};

const buildTypeHintsMap = structValue => {
	/** @type {{[key: string]: string}} */
	const typeHints = {};
	const rawTypeHints = structValue.constructor.TYPE_HINTS || {};
	Object.getOwnPropertyNames(rawTypeHints).forEach(key => {
		const hint = rawTypeHints[key];
		let ruleName;
		if (0 === hint.indexOf('array['))
			ruleName = hint;
		else if (0 === hint.indexOf('enum:'))
			ruleName = hint.substring('enum:'.length);
		else if (0 === hint.indexOf('pod:'))
			ruleName = hint.substring('pod:'.length);
		else if (0 === hint.indexOf('struct:'))
			ruleName = hint;

		if (ruleName)
			typeHints[key] = ruleName;
	});

	return typeHints;
};

const typeConverterFactory = (module, customTypeConverter, value) => {
	if (customTypeConverter && customTypeConverter(value))
		return customTypeConverter(value);

	if (value instanceof ByteArray)
		return new module[value.constructor.name](value.bytes);

	return value;
};

const autoEncodeStrings = entity => {
	Object.getOwnPropertyNames(entity).forEach(key => {
		const value = entity[key];
		if ('string' === typeof (value))
			entity[key] = new TextEncoder().encode(value);
	});
};

/**
 * Rule based transaction factory.
 * @note This class is not intended to be used directly.
 */
export default class RuleBasedTransactionFactory {
	/**
	 * Creates a rule based transaction factory for use with catbuffer generated code.
	 * @param {object} module Catbuffer generated module.
	 * @param {function|undefined} typeConverter Type converter.
	 * @param {Map<string, function>|undefined} typeRuleOverrides Type rule overrides.
	 */
	constructor(module, typeConverter = undefined, typeRuleOverrides = undefined) {
		/**
		 * @private
		 */
		this._module = module;

		/**
		 * Tries to coerce a value to a more appropriate type.
		 * @param {object} value Original value.
		 * @returns {object} Type converted value.
		 * @private
		 */
		this._typeConverter = value => typeConverterFactory(this._module, typeConverter, value);

		/**
		 * @private
		 */
		this._typeRuleOverrides = typeRuleOverrides || new Map();

		/**
		 * Map of rule names to transform functions.
		 * @type Map<string, function>
		 */
		this.rules = new Map();
	}

	/**
	 * Looks up a class in the wrapped module.
	 * @param {string} name Class name.
	 * @returns {Constructable} Class type.
	 * @private
	 */
	_getModuleClass(name) {
		return this._module[name];
	}

	/**
	 * Creates wrapper for SDK POD types.
	 * @param {string} name Class name.
	 * @param {Constructable} PodClass Class type.
	 */
	addPodParser(name, PodClass) {
		if (this._typeRuleOverrides.has(PodClass)) {
			this.rules.set(name, this._typeRuleOverrides.get(PodClass));
			return;
		}

		this.rules.set(name, value => (value instanceof PodClass ? value : new PodClass(value)));
	}

	/**
	 * Creates flag type parser.
	 * @param {string} name Class name.
	 */
	addFlagsParser(name) {
		const FlagsClass = this._getModuleClass(name);
		const stringToEnum = buildEnumStringToValueMap(FlagsClass);

		this.rules.set(name, flags => {
			if ('string' === typeof (flags)) {
				const enumArray = flags.split(' ').map(flagName => nameToEnumValue(stringToEnum, name, flagName));
				return new FlagsClass(enumArray.map(flag => flag.value).reduce((x, y) => x | y));
			}

			if ('number' === typeof (flags) && Number.isInteger(flags))
				return new FlagsClass(flags);

			return flags;
		});
	}

	/**
	 * Creates enum type parser.
	 * @param {string} name Class name.
	 */
	addEnumParser(name) {
		const EnumClass = this._getModuleClass(name);
		const stringToEnum = buildEnumStringToValueMap(EnumClass);

		this.rules.set(name, enumValue => {
			if ('string' === typeof (enumValue))
				return nameToEnumValue(stringToEnum, name, enumValue);

			if ('number' === typeof (enumValue) && Number.isInteger(enumValue))
				return new EnumClass(enumValue);

			return enumValue;
		});
	}

	/**
	 * Creates struct parser (to allow nested parsing).
	 * @param {string} name Class name.
	 */
	addStructParser(name) {
		const StructClass = this._getModuleClass(name);

		this.rules.set(`struct:${name}`, structDescriptor => {
			const structProcessor = this._createProcessor(structDescriptor);
			const structValue = new StructClass();

			const allTypeHints = buildTypeHintsMap(structValue);
			structProcessor.setTypeHints(allTypeHints);

			structProcessor.copyTo(structValue);
			return structValue;
		});
	}

	/**
	 * Creates array type parser, based on some existing element type parser.
	 * @param {string} name Class name.
	 */
	addArrayParser(name) {
		const elementRule = this.rules.get(name);
		if (!elementRule)
			throw Error(`cannot create array type parser because element rule "${name}" is unknown`);

		const elementName = name.replace(/^struct:/, '');
		this.rules.set(`array[${elementName}]`, values => values.map(value => elementRule(value)));
	}

	/**
	 * Autodetects rules using reflection.
	 */
	autodetect() {
		Object.getOwnPropertyNames(this._module).forEach(key => {
			const cls = this._module[key];
			if (Object.prototype.isPrototypeOf.call(BaseValue.prototype, cls.prototype))
				this.addPodParser(key, cls);
		});
	}

	/**
	 * Creates an entity from a descriptor using a factory.
	 * @param {function} factory Factory function.
	 * @param {object} descriptor Entity descriptor.
	 * @returns {object} Newly created entity.
	 */
	createFromFactory(factory, descriptor) {
		const processor = this._createProcessor(descriptor);
		const entityType = processor.lookupValue('type');
		const entity = factory(entityType);

		const allTypeHints = buildTypeHintsMap(entity);
		processor.setTypeHints(allTypeHints);
		processor.copyTo(entity, ['type']);

		autoEncodeStrings(entity);
		return entity;
	}

	/**
	 * Creates a transaction descriptor processor around a descriptor.
	 * @param {object} descriptor Transaction descriptor.
	 * @returns {TransactionDescriptorProcessor} Transaction descriptor processor.
	 * @private
	 */
	_createProcessor(descriptor) {
		return new TransactionDescriptorProcessor(descriptor, this.rules, this._typeConverter);
	}
}

// region type declarations

/**
 * Constructable class type.
 * @class
 * @typedef {{new(...args: any[]): object}} Constructable
 */

// endregion
