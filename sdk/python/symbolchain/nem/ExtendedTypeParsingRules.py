from ..CryptoTypes import Hash256, PublicKey
from ..TypeParserBuilder import TypeParserBuilder
from .Network import Address


def extend_type_parsing_rules(nem_type_converter, base_type_parsing_rules=None):
	builder = TypeParserBuilder('nc', nem_type_converter, base_type_parsing_rules)
	enum_parsers = {
		name: builder.create_enum_parser(name) for name in [
			'TransactionType', 'LinkAction', 'MosaicSupplyChangeAction', 'MosaicTransferFeeType',
			'MultisigAccountModificationType', 'MessageType'
		]
	}
	struct_parsers = {
		f'struct:{name}': builder.create_struct_parser(name, extend_type_parsing_rules) for name in [
			'Message', 'NamespaceId', 'MosaicId', 'Mosaic', 'SizePrefixedMosaic', 'MosaicLevy',
			'MosaicProperty', 'SizePrefixedMosaicProperty', 'MosaicDefinition',
			'MultisigAccountModification', 'SizePrefixedMultisigAccountModification'
		]
	}

	sdk_type_mapping = {
		'Address': Address,
		'Hash256': Hash256,

		# needed for things like AccountKeyLinkTransaction
		'PublicKey': PublicKey,
	}
	sdk_type_wrappers = {
		name: builder.create_sdk_wrapper(typename) for name, typename in sdk_type_mapping.items()
	}

	default_rules = {**enum_parsers, **struct_parsers, **sdk_type_wrappers}
	builder.add_base_value_rules(default_rules)

	final_rules = default_rules if not base_type_parsing_rules else {**default_rules, **base_type_parsing_rules}

	for name in ['SizePrefixedMosaic', 'SizePrefixedMosaicProperty', 'SizePrefixedMultisigAccountModification']:
		final_rules[f'array[{name}]'] = builder.create_object_array_parser(f'struct:{name}', final_rules)

	return final_rules
